/**
 * This code is to measure water quality with multiple sensors. It has been developed in
 * IoTLab Federation University, Churchill, Australia.
 * 
 * 
 */

#define TURBIDITY_SEN_ENABLE true
#define TDS_SEN_ENABLE true

#define TURBIDITY_SENSOR_PIN A0 
#define TDS_SENSOR_PIN A1

#define VREF 5.0             // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point

#define BURATE 115200
#define SAMPLE_POINTS 800

#define SETUP_DELAY 2000
#define SLEEPING_INTERVAL 1000

float voltage;
float sensorValue; 
float ntu;
float ntuReturn;

int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;
float tdsRet = 0;
float temperature = 25;

 
void setup(){// put your setup code here, to run once 
  Serial.begin(BURATE);
  
  // configure Turbidity sensor pin
#if TURBIDITY_SEN_ENABLE  
  pinMode(TURBIDITY_SENSOR_PIN, INPUT);
#endif

#if TDS_SEN_ENABLE
  pinMode(TDS_SENSOR_PIN, INPUT);
#endif
  
  delay(SETUP_DELAY);
}
 
void loop() {

// This part is for the Turbidity   
#if TURBIDITY_SEN_ENABLE  
  ntuReturn = getTurbidity();
  Serial.print("Water Turbidity value: ");
  Serial.print(ntuReturn);
  Serial.println(" ntu");
#endif

// This part is for Total Dissolved Solids (TDS)
#if TDS_SEN_ENABLE
  tdsRet = getTDS();
  Serial.print("TDS Value: ");
  Serial.print(tdsRet,0);
  Serial.println(" ppm");
#endif

  delay(SLEEPING_INTERVAL);
  
  Serial.println("");
}

float getTDS() {
  
  while(1) {
    
    // put your main code here, to run repeatedly:
     static unsigned long analogSampleTimepoint = millis();
     if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
     {
       analogSampleTimepoint = millis();
       analogBuffer[analogBufferIndex] = analogRead(TDS_SENSOR_PIN);    //read the analog value and store into the buffer
       analogBufferIndex++;
       if(analogBufferIndex == SCOUNT) 
           analogBufferIndex = 0;
     }   
     
     static unsigned long printTimepoint = millis();
     
     if(millis()-printTimepoint > 800U)
     {
        printTimepoint = millis();
        for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
          analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
        averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
        float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
        float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
        tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
  
        return tdsValue; 
     }
  }
}

int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

float getTurbidity() {
  sensorValue = 0;
  
  for(int i = 0; i < SAMPLE_POINTS; i++) {
    sensorValue += ((float) analogRead(TURBIDITY_SENSOR_PIN) / 1023) * 5;  
  }
  
  voltage = sensorValue / SAMPLE_POINTS;
  voltage = round_to_dp(voltage, 2);
  
  
  if(voltage < 2.5) {
    ntu = 3000;
  } else {
    ntu = -1120.4 * sqrt(voltage) + 5742.3 * voltage - 4352.9;     
  }
  
  return ntu;  
}


float round_to_dp(float in_value, int decimal_place) {
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}
