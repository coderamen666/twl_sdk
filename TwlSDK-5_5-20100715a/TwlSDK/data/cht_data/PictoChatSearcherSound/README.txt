******************************************************
	Sound Data for the PictoChat Searcher
    
******************************************************
The sound effect data played when the PictoChat Searcher displays an icon is called by this package (folder).
Besides these, there are no other specific sound effects (for things like opening windows or choosing to run Chat), so play other sound effects that are being used in the game as needed.
Breakdown of Files
BankPCS/sePCS.bnk
A buffer file in which illustrations to be used are defined
SeqPCS/sePCS.mus
A sequence archive file in which sound effect sequences are written
WavePCS/CelestaPost.32.t.an5.aiff
Source waveform
sound_data.sarc
Sound archive
MakeSound.bat
ReMakeSound.bat
A batch file to make sound data
Incorporation into Applications
There is an easy method for incorporating this data into each game project.
The main methods are shown below.
(Other methods of inclusion are acceptable.)
Note: Each label in its initial state is as follows.
Sequence archive - - - SEQARC_SE_PCS
Bank - - - - - - - - - - BANK_SE_PCS
Waveform archive - - - - - - WAVE_SE_PCS
* Method #1
Use sound data created by this package (sound_data.sdat).
This method creates sound data (sound_data.sdat) using MakeSound.bat, puts that data on the ROM, then plays sounds jointly with the regular sound data in the game.
Keeping the data independent has the advantage of making it easy to work with, but because you have to change the sound data being used before calling the function that also plays game sound effects, that will have to be addressed when data is included.
* Method #2
The bank and sequence archive must be registered in the sound archive (sound_data.sarc) of each game.
By adding to each game&rsquo;s sound archive, it is possible to play and manage data the same way you would the game&rsquo;s sound data. The bank used in the sequence archive is set to reference sound_data.sbdl, so if the path between this file and the sequence archive (sePCS.mus) changes, you must rewrite the include path.
* Method #3
Copy and paste to the files that use the contents of the text files in the bank and sequence archive.
In Method #2, data that is not part of the game is being handled, so the total size increases a bit.
To avoid this, the text base of each piece of data must be copied and pasted.
Notes
* Parameter changes
The player numbers and priority of sound effects in the sequence archive are set, so you have to make changes depending on the conditions in the game.
Change player numbers to those which are not used by the game, and coordinate priorities to keep a balance with those used in the game.
* Path confirmation and changes
When changing a file&rsquo;s path, each text file needs to be adjusted.
* Checking original sound effects
If you make a mistake when incorporating the data, there is a possibility that a sound different from the original one will be played back.
Always run SoundPlayer.srl, created by the batch file, to check the original sounds once.
