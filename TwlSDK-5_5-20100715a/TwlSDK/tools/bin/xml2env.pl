#! perl
#---------------------------------------------------------------------------
#  Project:  TwlSDK - xml2env
#  File:     xml2env.pl
#
#  Copyright 2005-2008 Nintendo. All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law. They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date:: 2008-09-18#$
#  $Rev: 8573 $
#  $Author: okubata_ryoma $
#---------------------------------------------------------------------------


use strict;

# Check availability of XML::Parser
if (eval "require XML::Parser; return 1;" != 1){
    printf "Use cygwin setup to insert the newest version of Perl and the expat library. Set up an environment that allows you to use XML::Parser. Then run again. \n";
    exit;
}

require Getopt::Std;
require XML::Parser;

# Specify access to the handler routine as a parameter and initialize the parser
my $parser = XML::Parser -> new(Handlers => {
        Init =>    \&handle_doc_start,
        Final =>   \&handle_doc_end,
        Start =>   \&handle_elem_start,
        End =>     \&handle_elem_end,
        Char =>    \&handle_char_data,
});
   
my (%opts);
my $c = Getopt::Std::getopts('o:h', \%opts);

my ($src, $tmp, $line, $resource_name, $resource_count);

# Output filename used when no options have been attached
$tmp = $src . '.result.c';


# h for help
if(exists $opts{h} || scalar(@ARGV) < 1){
    print "After the command enter one argument (the .xml file you want to search) \n";
    print "Input example: perl xml2env.pl d:/test/main.xml \n";
    print "In the above example, the file is in d:/test/ \n";
    print "If in the argument you enter "-o\" followed by a file name, the script will output to the file. \n";
    print "If you enter "-h\" in the argument, a simple help menu will be displayed. \n";
    die "$!";
}



# Enter name of output file after the -o option
if(exists $opts{o}){
    $tmp = $opts{o};
}

# Enter the name of the file to be accessed without an option
if(@ARGV){
    $src = $ARGV[$resource_count];
}

open(FO, ">" . $tmp) or die ("cannot open $tmp to write.");

my %default_type = (
 "u8"         => "ENV_U8",
 "s8"         => "ENV_S8",
 "u16"        => "ENV_U16",
 "s16"        => "ENV_S16",
 "u32"        => "ENV_U32",
 "s32"        => "ENV_S32",
 "u64"        => "ENV_U64",
 "s64"        => "ENV_S64",
 "bool"       => "ENV_BOOL",
 "string"     => "ENV_STRING",
 "hexbinary"  => "ENV_BINARY"
);

$line = "/*---------------------------------------------------------------------------*/\n";
$line .= "#include <nitro.h>\n";
$line .= "//================================================================================\n";
$line .= "\n";
print   FO  $line;


# Read data and parse
my $file = $src;
if($file){
    $parser->parsefile($file);
}
else{
    my $input = "";
    while(<STDIN>){ $input .= $_; }
    $parser->parse($input);
}

close(FO);
exit;

my ($record, $context, $resource_names, $datafile_name);

# Handler

# Output the top of the file simultaneously as processing starts
sub handle_doc_start{
    print "";
}
my ($class_name, $class_context);
# Parser is called each time the launch of a new element is encountered
sub handle_elem_start{
    my($expat, $name, %atts) = @_;
    $context = $name;
    
    if($name =~ /resource/i){
        $resource_names .= " $atts{'name'},";
        print "ENVResource $atts{'name'}\[\] = {\n";
        print   FO  "ENVResource $atts{'name'}\[\] = {\n";
    }
    elsif($name =~ /class/i){
        $class_name = $atts{'name'};
    }
    else{
        # In the loop, search for items coming under default_type and process
        foreach my $i(keys %default_type){
            if($name =~ /$i/i){
                $class_context = "\"$class_name\.$atts{'name'}\",\t". "$default_type{$name}";
                if($name =~ /hexbinary/i){
                    if(exists $atts{'src'}){
                        $datafile_name = $atts{'src'};
                        # Initialize attribute values used when loading files
                        $atts{'src'} = undef;
                    }
                }
            }
        }
    }
}

# Store element-internal character data in the latest element buffer
sub handle_char_data{
    my($expat, $text) = @_;
    
    $record->{$context} .= $text;
    
}
my ($line, $filedata, $char);
# Call the parser each time the close of a new element is encountered
sub handle_elem_end{
    my($expat, $name) = @_;
    if($name =~ /resource/i){
        print "    ENV_RESOURCE_END\n};\n\n";
        print   FO  "    ENV_RESOURCE_END\n};\n\n";
    }
    # When the element name is not 'class'
    elsif($name ne 'class'){
        if($name =~ /hexbinary/i){
            # When the element name is 'hexbinary' and it is necessary to load a file
            if($datafile_name){
                open(FP, "<" . $datafile_name) or die ("cannot open $datafile_name to read.");
                while(!eof(FP)) {
                    $char = getc FP;
                    # Convert characters to ASCII code
                    $char = unpack("C", $char);
                    # Convert ASCII code to hexadecimal
                    $char = sprintf("%x", $char);
                    $filedata .= '\x' . $char;
                }
                print "    $class_context( \"$filedata\" ),\n";
                print   FO  "    $class_context( \"$filedata\" ),\n";
                # Initialize file read flag
                $datafile_name = undef;
                $filedata = "";
            }
            else{
                my @ascii;
                my $hexdata;
                # Convert characters to ASCII code 
                @ascii = unpack("C*", $record->{$context});
                foreach my $i(@ascii){
                    # Convert ASCII code to hexadecimal
                    $hexdata .= '\x' . sprintf("%x", $i);
                }
                print "    $class_context( \"$hexdata\" ),\n";
                print   FO  "    $class_context( \"$hexdata\" ),\n";
                # Initialize text each time
                $record->{$context} = "";
            }
        }
        elsif($name =~ /string/i){
            $record->{$context} =~ s/(["\\])/\\$1/g;
            $record->{$context} =~ s/[\n]/\\n/g;
            $record->{$context} =~ s/[\t]/\\t/g;
            print "    $class_context( \"$record->{$context}\" ),\n";
            print   FO  "    $class_context( \"$record->{$context}\" ),\n";
            # Initialize text each time
            $record->{$context} = "";
        }
        else{
            # In the loop, search for items coming under default_type and process
            foreach my $i(keys %default_type){
                if($name =~ /$i/i){
                    print "    $class_context( $record->{$context} ),\n";
                    print   FO  "    $class_context( $record->{$context} ),\n";
                    # Initialize text each time
                    $record->{$context} = "";
                }
            }
        }
        # Initialize text
        $class_context = "";
        $context = "";
    }
}

# Output end of file last
sub handle_doc_end{
    print "ENVResource* resourceArray\[\]={";
    print   FO  "ENVResource* resourceArray\[\]={";
    print "$resource_names";
    print   FO  "$resource_names";
    print " NULL };";
    print   FO  " NULL };";
}

