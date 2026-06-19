/******************************************************************************
  Uses elements of by Jeremy Cook June, 2026

  Example1_BasicReadings.ino
  
  https://github.com/sparkfun/SparkFun_STHS34PF80_Arduino_Library

  Development environment specifics:

  IDE: Arduino 2.2.1
  Hardware Platform: SparkFun RedBoard Qwiic	
  SparkFun Human Presence and Motion Sensor - STHS34PF80 (Qwiic) Version: 1.0
  SparkFun Qwiic Mini Human Presence and Motion Sensor - STHS34PF80 Version: 1.0

  Do you like this library? Help support SparkFun. Buy a board!

    SparkFun Human Presence and Motion Sensor - STHS34PF80 (Qwiic)
    https://www.sparkfun.com/products/22494
    
    SparkFun Qwiic Mini Human Presence and Motion Sensor - STHS34PF80
    https://www.sparkfun.com/products/23253

  Hardware Connections:
  Use a Qwiic cable to connect from the RedBoard Qwiic to the STHS34PF80 breakout (QWIIC).
  You can also choose to wire up the connections using the header pins like so:

  ARDUINO --> STHS34PF80
  SDA (A4) --> SDA
  SCL (A5) --> SCL
  3.3V --> 3.3V
  GND --> GND

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "SparkFun_STHS34PF80_Arduino_Library.h"
#include <Wire.h>

STHS34PF80_I2C mySensor;

// Values to fill with presence and motion data
int16_t presenceVal = 0;
int16_t motionVal = 0;
float temperatureVal = 0;

static int redLight = 21;
static int greenLight = 22;
static int speakerPin = 14;
static int presenceThreshold = 500;
static int topMotionThreshold = 800;
static int botMotionThreshold = -800;


void setup()
{
    Serial.begin(115200);
    Serial.println("STHS34PF80 LED/tone indicator");

    // Begin I2C
    Wire.begin();

    // Establish communication with device 
    if(mySensor.begin() == false)
    {
      Serial.println("Error setting up device - please check wiring.");
      while(1);
    }

    pinMode(redLight, OUTPUT);
    pinMode(greenLight, OUTPUT);

    delay(1000);
}

void loop()
{
  sths34pf80_tmos_drdy_status_t dataReady;
  mySensor.getDataReady(&dataReady);

  // Check whether sensor has new data - run through loop if data is ready
  if(dataReady.drdy == 1)
  {
    sths34pf80_tmos_func_status_t status;
    mySensor.getStatus(&status);
    
    // If presence flag is high, then print data
    if(status.pres_flag == 1)
    {
      // Presence Units: cm^-1
      mySensor.getPresenceValue(&presenceVal);
      Serial.print("Presence: ");
      Serial.print(presenceVal);
      Serial.println(" cm^-1");
    }

    if(presenceVal > presenceThreshold){
      digitalWrite(redLight, HIGH);
    }
    else digitalWrite(redLight, LOW);

    if(status.mot_flag == 1)
    {
      mySensor.getMotionValue(&motionVal);
      Serial.println("Motion Detected! ");
      Serial.print("MotionValue: ");
      Serial.println(motionVal);
      }
    if((status.mot_flag == 1) && ((motionVal > topMotionThreshold) or (motionVal < botMotionThreshold))){
      digitalWrite(greenLight, HIGH);
      tone(speakerPin, 500);
    }
    else{
      digitalWrite(greenLight, LOW);
      noTone(speakerPin);
    } 

  }
      
}

