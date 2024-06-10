// demonstration, how the data will be packed and ready to send
// Edge impulse -> data collection -> Data preprocessing -> Model training -> Model deployment -> Classification

// in our code we will do like this: 
// Data recording -> Signal preparation -> Classification -> Result handling
// microphone_inference_record() -> struct signal_t  -> run_classifier_continuous() -> print_results & send via LoRa module  // in slices.. 1 slice has 12k/slices per window = 3k originally
// slices are segments or chunks of the input data, we have double buffering

// classifier = buzz / no buzz

// buzz / no buzz has confidence level ( stopnja zaupanja ) between 0 and 1
#pragma pack(push, 1)  // delete padding 
typedef struct
{

  uint8_t version;
  uint8_t hum;                    // 0 -> 100% humidity

  uint16_t buzzConfidenceLevel;   // (float) 0 -> 1 to integer 
  uint16_t nobuzzConfidenceLevel; // (float) 0 -> 1 to integer 

  uint16_t temp;                  // 0 -> 100 , 1 decimal place
  uint16_t pressure;              // 300 -> 1100 hPa

} Bees_data_t;
#pragma pack(pop) 

Bees_data_t Bees;               // declare the structure
Bees_data_t *Bees_ptr = &Bees;  // declare the pointer to the structure

void CalcBeforeSending() {
    if (!Bees.version) {
        Bees.version = VERSION;                                                            // we will have e.g. #DEFINE VERSION 1
    }

    // humidity calculation
    Bees.hum = Humidity;                                                                   // e.g Humidity = bme.humidity
    
    // buzz calculation
    Bees.buzzConfidenceLevel = CalculateConfidenceLevel(result.classification[0].value);   // buzz confidence level
    Bees.nobuzzConfidenceLevel = CalculateConfidenceLevel(result.classification[1].value); // nobuzz confidence level

    // temperature calculation
    Bees.temp *=10;                     // temperature will be in ragne 0-> 100, with 1 decimal point
    Bees.temp = (uint16_t) Bees.temp;

    //pressure calculation
    Bees.pressure = Pressure;           // e.g Pressure = bme.pressure         


    PrintCalculatedData();              // display the data 
    //MySerialPrint(Bees);                // display the data in HEX

}


void PrintCalculatedData() {
    Serial.print("\n Verzija programa = ");
    Serial.println(Bees.version);
    Serial.print("\n vrednost vlage = ");
    Serial.println(Bees.hum);
    Serial.print("\n vrednost stopnje zaupanja, da je cebela .. Buzz = ");
    Serial.println(Bees.buzzConfidenceLevel);
    Serial.print("\n vrednost stopnje zaupanja, da ni cebela Nobuzz= ");
    Serial.println(Bees.buzzConfidenceLevel);
    Serial.print("\n vrednost temperature = ");
    Serial.println(Bees.temp);
    Serial.print("\n vrednost tlaka = ");
    Serial.println(Bees.pressure);

}

int CalculateConfidenceLevel(float x) {
    // Check if input is within the valid range
    if (x < 0.0 || x > 1.0) {
        Serial.print("Input must be within the range [0, 1]\n");
        return -1;                      // Return an error value
    }
    return (int)(x * 10000);            // return the integer value of confidence level
}

// custom made serial print( template function ), so we can print every datatype of data while debugging
// be aware, that the structure could be LE or BE 
// use e.g :    MySerialPrint(*Bees_ptr) or MySerialPrint(Bees)
template<typename T> unsigned int MySerialPrint(const T &value) {
  const byte *p = (const byte *)&value;
  unsigned int i;
  for (i = 0; i < sizeof value; i++)
    Serial.println(*p++, HEX);
  return i;
}
