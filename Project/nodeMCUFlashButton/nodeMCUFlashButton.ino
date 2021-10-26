
#define LED_BUILTIN 2
#define BUTTON_BUILTIN 0 
void setup() {
  // El LED integrado está conectado al pin 2.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_BUILTIN, INPUT);
} 

void loop() {
  // Leer el estado del botón. Cuando está pulsado el pin se pone a nivel bajo<br /><br /><br />
  int estado = digitalRead(BUTTON_BUILTIN);
  // Configurar el LED de acuerdo al estado del botón
  digitalWrite(LED_BUILTIN, estado);
}
