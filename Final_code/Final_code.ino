
#define pulsePin A0 // Analog input pin.


//  VARIABLES
//int rate[10];   
int BPM = 0;                 
int Signal = 0;
int threshold = 600;
int current_value = 0;
int previous_value = 0;
int peak = 512;
int trough = 550;  
bool peak_flag = false;
bool trough_flag = true;
int peak_time = 0;
int past_peak_time = 0;
int IBI = 0;
unsigned long previousMillis = 0; 

const long interval_1 = 6000;
bool Start_flag = false;
bool past_peak_time_flag = false;

// Circular Buffer implementation.
struct CircularBuffer{
  CircularBuffer(size_t size);
  void addElement(float newElement);
  float average();
  
  float* array;
  float* next;
  size_t size;
  int index = 0;
  int num_elements = 0;
};

CircularBuffer::CircularBuffer(size_t arraySize)
{
  size = arraySize;
  array = new float[arraySize];
  next = &array[0];
}

void CircularBuffer::addElement(float newElement)
{
  *next = newElement;
  index++;
  if (index >= size)
  {
    index = 0;
  }
  next = &array[index];
  if(++num_elements > size)
  {
    num_elements = size;
  }
}

float CircularBuffer::average()
{
  float average = 0;
  for(int i = 0; i < num_elements; i++)
  {
    average += array[i];
  }
  return average/num_elements;
}

// circular buffer of size 10 that holds peak values which is averaged to calculate heart rate  implementation 

CircularBuffer myBuffer(10);  // create a 10 element (float) circular buffer


void setup() {
  Serial.begin(9600);

}

void loop() {
            Signal = analogRead(pulsePin);            // Read the analog pin
            if (Signal < 250) {                       // whenever there is a finger on the sensor the signal drops, this case is to recognise when the finger is placed on the sensor
              Serial.println("signal is < 250");
              Start_flag = true;                      // set the start_flag
            }
            if (Start_flag == true){                  
              Peak_trough();                          // calculate the peaks and eventually compute Heart rate 
            }
            
            if (Start_flag == false){                 // this flag is set when there no peak for certain interval
              past_peak_time_flag = false;            // this flag is reset and set to true when there is a finger on the sensor. 
              Serial.println("no pulse");
            }
            delay(50);
}


void Peak_trough() {
  if (Signal > threshold){                                    // checks if the signal is greater than threshold
    if (Signal < previous_value && peak_flag == false){       // checks if the signal value is less than previous peak.  
      peak_flag = true;                                       // the peak is found
      trough_flag = false;                                    // trough flag is set to false
      peak = previous_value ;                                 // the peak value is set to previous value 
      peak_time = millis();                             
      IBI = peak_time - past_peak_time;                       // calculate the interval between two peaks. 
      if (IBI < 1500){
        Serial.print("IBI: ");
        Serial.println(IBI);
        myBuffer.addElement(IBI);                             // Add the element to the circular buffer
        BPM = myBuffer.average();                             // average the circular buffer
        
        BPM = 60000/BPM;                                      // calculate the Beats per minute 
        Serial.println();
        Serial.print("BPM: ");
        Serial.println(BPM);
        past_peak_time_flag = true;                           // Keep this flag true as long as the finger is on the sensor. 
      }

    }
  }
  past_peak_time = peak_time;                                 // keep track of the past peak value
  
  if (Signal < threshold){                                    // check if the signal is less than threshold to look for trough
    if (Signal > previous_value && trough_flag == false) {    // if the signal at present time point is greater than past time point then the value at the past timepoint is the trough value. 
      peak_flag = false;                                      // set the peak flag to false
      trough_flag = true;                                     // set the trough flag to true
      trough = previous_value;                                // track the trough value 
    }
  }  
  previous_value = Signal;                                    

  

  unsigned long currentMillis_1 = millis();
  if (currentMillis_1 - past_peak_time >= interval_1 && past_peak_time_flag == true) { // if the time interval is greater than threshold, there is no finger on the sensor
    Serial.println("Start_flag is < false");
    Start_flag = false;                                                                   
    threshold = 600;                                                                   // reset the threshold.
      
  }
}
