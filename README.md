InoPinTester
==========

[Arduino](http://arduino.cc) sketch for testing Arduino I/O functions via serial commands. Developed for testing custom variant files.


#### Instructions
- Download the most recent version of InoPinTester here: https://github.com/per1234/InoPinTester/archive/master.zip
- Extract the InoPinTester-master folder from the downloaded zip file.
- Move the folder to a convenient location.
- Open InoPinTester/InoPinTester.ino in the Arduino IDE.
- Upload the sketch to your board.
- Open the Serial Monitor(magnifying glass button in the top right of the Arduino IDE window).
- Select **No line ending** from the menu in the lower right of the Serial Monitor window.
- Select **9600 baud** in the menu in the lower right of the Serial Monitor window.
- Commands will be listed in the Serial Monitor. Commands can be sent via the text input box at the top of the Serial Monitor. In addition, you can enter pin numbers.
  - NOTE: If you are using a board that doesn't have native USB and doesn't reset when Serial Monitor is opened (e.g., Nano Every), you will not see the command menu. Type `h` in the Serial Monitor input box and press Enter to display the menu.


#### Contributing
Pull requests or issue reports are welcome! Please see the [contribution rules](https://github.com/per1234/InoPinTester/blob/master/.github/CONTRIBUTING.md) for instructions.
