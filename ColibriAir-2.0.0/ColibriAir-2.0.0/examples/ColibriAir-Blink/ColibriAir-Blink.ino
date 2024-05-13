/*                                                                                 
     _/_/_/            _/  _/  _/                  _/        _/_/    _/            
  _/          _/_/    _/      _/_/_/    _/  _/_/          _/    _/      _/  _/_/   
 _/        _/    _/  _/  _/  _/    _/  _/_/      _/      _/_/_/_/  _/  _/_/        
_/        _/    _/  _/  _/  _/    _/  _/        _/      _/    _/  _/  _/           
 _/_/_/    _/_/    _/  _/  _/_/_/    _/        _/      _/    _/  _/  _/            
                                                                                   
  ColibriAir - Blink
*/

// Deklaracija spremenljivk

// Določitev pinov kamor so vezane LED
const int LED_BLUE = 25;
const int LED_GREEN = 26;
const int LED_RED = 27;


// Funkcija Setup se izvede le enkrat ko resetiramo napravo
void setup() {
  
  // Inicializacija digitalnih izhodov
  pinMode(LED_GREEN, OUTPUT);  
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);


  // Vse LED ugasnjene
  digitalWrite(LED_GREEN, HIGH); 
  digitalWrite(LED_BLUE, HIGH); 
  digitalWrite(LED_RED, HIGH);        

}

// Funkcija Loop se izvaja vedno znova in znova
void loop() {

  digitalWrite(LED_GREEN, HIGH);    // Ugasni Zeleno LED
  digitalWrite(LED_RED, LOW);       // Prižgi Rdečo LED                  
  delay(500);                       // Počakaj pol sekunde (500 ms)  
  digitalWrite(LED_RED, HIGH);      // Ugasni Rdečo LED  
  digitalWrite(LED_BLUE, LOW);      // Prižgi Modro LED     
  delay(500);                       // Počakaj pol sekunde (500 ms)      
  digitalWrite(LED_BLUE, HIGH);     // Ugasni Modro LED  
  digitalWrite(LED_GREEN, LOW);     // Prižgi Zeleno LED  
  delay(500);                       // Počakaj pol sekunde (500 ms)  
  
}
