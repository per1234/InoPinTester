// Use serial commands to control digitalWrite(), digitalRead(), analogWrite(), analogRead(), attachInterrupt() and pin change interrupt of Arduino pins

volatile bool interruptTriggered = false;
volatile bool pinChangeInterruptTriggered = false;
volatile byte pinChangeInterruptNumber = 42;

enum __attribute__((packed)) testMode_t{
  modeNone,
  modeDigitalRead,
  modeDigitalWrite,
  modeAnalogRead,
  modeAnalogWrite,
  modeExternalInterrupt,
#if defined(digitalPinToPCICR)
  modePinChangeInterrupt
#endif  //defined(digitalPinToPCICR)
};

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(100);
  while (!Serial) {}  //on boards w/ native USB, wait until Serial Monitor is opened before printing the command list
  printHelp();
}


void loop() {
  static testMode_t mode = modeNone;
  static byte pin = 2;
  static unsigned int interval;
  static bool interruptAttached = false;
#if defined(digitalPinToPCICR)
  static bool pinChangeInterruptAttached = false;
#endif  //defined(digitalPinToPCICR)

  //handle serial input
  if (Serial.available()) {
    const testMode_t previousMode = mode;
    const byte previousPin = pin;
    const char command = Serial.peek();

    Serial.print(F("Command: "));
    if (command >= '0' && command <= '9') {
      //pin number specified
      pin = Serial.parseInt();
      Serial.println(pin);
    }
    else {
      Serial.println(command);

      if (command == 'r' || command == 'R') {
        mode = modeDigitalRead;
        interval = 500;
      }
      else if (command == 'w' || command == 'W') {
        interval = 150;
        mode = modeDigitalWrite;
      }
      else if (command == 'a' || command == 'A') {
        interval = 500;
        mode = modeAnalogRead;
      }
      else if (command == 'p' || command == 'P') {
        interval = 1;
        mode = modeAnalogWrite;
      }
      else if (command == 'i' || command == 'I') {
        interval = 0;
        mode = modeExternalInterrupt;
      }
#if defined(digitalPinToPCICR)
      else if (command == 'c' || command == 'C') {
        interval = 0;
        mode = modePinChangeInterrupt;
      }
#endif  //defined(digitalPinToPCICR)
      else if (command == 'h' || command == 'H') {
        printHelp();
        mode = modeNone;
      }
      else if (command == 'n' || command == 'N') {
        //go to the next pin
        pin++;
      }
      else {
        Serial.println(F("Command not recognized"));
      }
    }

    //clear the Serial input buffer
    while (Serial.available()) {
      Serial.read();
    }

    if (pin != previousPin) {
      pinMode(previousPin, INPUT);  //return previous pin to the safest state
      Serial.print(F("Pin: "));
      Serial.println(pin);
    }

    if (pin != previousPin || mode != previousMode) {
#ifdef AVR
      detachInterrupt(digitalPinToInterrupt(pin));
#else
      detachInterrupt(pin);
#endif
      interruptAttached = false;

#if defined(digitalPinToPCICR)
#ifdef AVR
      //disable all pin change interrupts
#ifdef PCMSK0
      PCMSK0 = 0;
#endif //PCMSK0
#ifdef PCMSK1
      PCMSK1 = 0;
#endif //PCMSK1
#ifdef PCMSK2
      PCMSK2 = 0;
#endif //PCMSK2
#ifdef PCMSK3
      PCMSK3 = 0;
#endif //PCMSK3
#ifdef PCMSK4
      PCMSK4 = 0;
#endif //PCMSK4
#ifdef PCMSK5
      PCMSK5 = 0;
#endif //PCMSK5
#ifdef PCMSK6
      PCMSK6 = 0;
#endif //PCMSK6
#ifdef PCMSK7
      PCMSK7 = 0;
#endif //PCMSK7
      //clear outstanding pin change interrupts
      PCIFR = 0;
#endif  //AVR
      pinChangeInterruptAttached = false;
#endif  //defined(digitalPinToPCICR)
    }
  }

  static unsigned long timestamp = 0;
  if (millis() - timestamp > interval) {
    timestamp = millis();
    switch (mode) {
      case modeDigitalRead:
        pinMode(pin, INPUT_PULLUP);

        Serial.print(F("digitalRead("));
        Serial.print(pin);
        Serial.print(F(") = "));
        Serial.println(digitalRead(pin) == LOW ? F("LOW") : F("HIGH"));
        break;
      case modeDigitalWrite:
        pinMode(pin, OUTPUT);
        static byte state;
        if (state == LOW) {
          state = HIGH;
        }
        else {
          state = LOW;
        }
        digitalWrite(pin, state);
        break;
      case modeAnalogRead:
        pinMode(pin, INPUT_PULLUP);

        Serial.print(F("analogRead("));
        Serial.print(pin);
        Serial.print(F(") = "));
        Serial.println(analogRead(pin));
        break;
      case modeAnalogWrite:
        pinMode(pin, OUTPUT);
        static byte increment = 1;
        state += increment;
        if (state <= 0 || state >= 255) {
          increment = -increment;
        }

        analogWrite(pin, state);
        break;
      case modeExternalInterrupt:
        if (!interruptAttached) {
          interruptAttached = true;
#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAMD_BETA)
          if (g_APinDescription[pin].ulExtInt == NOT_AN_INTERRUPT)
#else //defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAMD_BETA)
          if (digitalPinToInterrupt(pin) == NOT_AN_INTERRUPT)
#endif  //defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAMD_BETA)
          {
            Serial.print(F("Pin "));
            Serial.print(pin);
            Serial.println(F(" is not an interrupt pin."));
          }
          else {
            pinMode(pin, INPUT_PULLUP);
#ifdef AVR
            attachInterrupt(digitalPinToInterrupt(pin), interruptFunction, CHANGE);
#else //AVR
            attachInterrupt(pin, interruptFunction, CHANGE);
#endif  //AVR
            interruptTriggered = false;
          }
        }
        if (interruptTriggered) {
          Serial.println(F("Interrupt triggered."));
          interruptTriggered = false;
        }
        break;
#if defined(digitalPinToPCICR)
      case modePinChangeInterrupt:
        interval = 0;
        if (!pinChangeInterruptAttached) {
          pinChangeInterruptAttached = true;
          if (digitalPinToPCICR((int8_t)pin) == 0) {
            Serial.print(F("Pin "));
            Serial.print(pin);
            Serial.println(F(" is not a pin change interrupt pin."));
          }
          else {
            pinMode(pin, INPUT_PULLUP);
            //enable pin change interrupts for the pin
            //https://playground.arduino.cc/Main/PinChangeInterrupt
            *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
            PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
            PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
          }
        }
        if (pinChangeInterruptTriggered) {
          Serial.print(F("Pin change interrupt number "));
          Serial.print(pinChangeInterruptNumber);
          Serial.println(F(" triggered."));
          pinChangeInterruptTriggered = false;
        }
        break;
#endif  //defined(digitalPinToPCICR)
      case modeNone:
        break;
    }
  }
}


void printHelp() {
#if defined(digitalPinToPCICR)
  Serial.println(F("digital[R]ead, digital[W]rite, [a]nalogRead, analogWrite ([P]WM), External [I]nterrupt, Pin [C]hange Interrupt, [N]ext pin, [H]elp"));
#else //defined(digitalPinToPCICR)
  Serial.println(F("digital[R]ead, digital[W]rite, [a]nalogRead, analogWrite ([P]WM), External [I]nterrupt, [N]ext pin, [H]elp"));
#endif  //defined(digitalPinToPCICR)
}

//function called by interrupt ISR
void interruptFunction() {
  interruptTriggered = true;
}

//pin change interrupt ISRs
#ifdef AVR
#ifdef PCINT0_vect
ISR (PCINT0_vect) {
  pinChangeInterruptTriggered = true;
  pinChangeInterruptNumber = 0;
}
#endif  //PCINT0_vect

#ifdef PCINT1_vect
ISR (PCINT1_vect) {
  pinChangeInterruptTriggered = true;
  pinChangeInterruptNumber = 1;
}
#endif  //PCINT1_vect

#ifdef PCINT2_vect
ISR (PCINT2_vect) {
  pinChangeInterruptTriggered = true;
  pinChangeInterruptNumber = 2;
}
#endif  //PCINT2_vect

#ifdef PCINT3_vect
ISR (PCINT3_vect) {
  pinChangeInterruptTriggered = true;
  pinChangeInterruptNumber = 3;
}
#endif  //PCINT3_vect

#ifdef PCINT4_vect
ISR (PCINT4_vect) {
  pinChangeInterruptTriggered = true;
  pinChangeInterruptNumber = 4;
}
#endif  //PCINT0_vect

#ifdef PCINT5_vect
ISR (PCINT5_vect) {
  pinChangeInterruptTriggered = true;
  pinChangeInterruptNumber = 5;
}
#endif  //PCINT5_vect

#ifdef PCINT6_vect
ISR (PCINT6_vect) {
  pinChangeInterruptTriggered = true;
  pinChangeInterruptNumber = 6;
}
#endif  //PCINT6_vect

#ifdef PCINT7_vect
ISR (PCINT7_vect) {
  pinChangeInterruptTriggered = true;
  pinChangeInterruptNumber = 7;
}
#endif  //PCINT7_vect
#endif  //AVR
