MB Library Multiboot Parent Demo: Operation Summary 

When the program starts, the parent device starts automatically, and the following list is displayed.

AID USERNAME STATE
 1  name1    NONE
 2  name2    NONE
 3  name3    NONE
 4  name4    NONE
....

 

Using the +Control Pad, select the child device that will become the target of operations and then press the A Button to perform the operation corresponding to the STATE.
Or, use START to perform the same operations as the A Button, but on all connected child devices.

Pressing SELECT cancels and restarts wireless communications.

Explanation of Each STATE

NONE          ... No connection
CONNECTED     ... Connection made from a child device
DISCONNECTED  ... Child device has cut the connection
ENTRY REQUEST ... Entry request has been made from a child device
                  [A Button: Allow entry.  B Button: Disallow entry]
KICKED        ... Child device has been kicked
ENTRY OK      ... A download request from a child device has been allowed
                  [A Button: Start binary send]
SENDING       ... Sending binary to a child device
SEND COMPLETE ... Binary send to the child device has finished
                  [A Button: sends a boot request]
BOOT REQUEST  ... Starts to send a boot request to a child device
BOOT READY    ... Download is complete, and the downloaded application is ready to run
MEMBER FULL   ... Child devices cannot connect to a game that has reached its set number of players

The operations shown in square brackets can be automated by setting the following #define statements to 1.

#define AUTO_ACCEPT_ENTRY	(0)
#define AUTO_SENDFILE		(0)
#define AUTO_BOOTREQ		(0)


  Note: This program supports only the display of ASCII characters, so the user names and others in the current child device binary mb_child_NITRO.srl and mb_child_TWL.srl are not displayed correctly.
   Please be aware beforehand.


