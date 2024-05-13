## Hannah Haggerty and Alyssa Aragon

#### Project Description
The swamp cooler system is designed to manage temperature and water levels within the cooler. The system relies on different components that work together to achieve its cooling functionality. The swamp cooler functionalities are when the temperature surpasses a certain threshold, the fan activates, triggering the blue LED to illuminate. If there's enough water, the fan turns on and runs until the desired temperature is met. The red LED indicates low water levels, prompting an error message on the LCD. Additionally, the LCD displays temperature and humidity readings for the user.

#### Components
- **Arduino Microcontroller:** Coordinates the operation of sensors and actuators.
- **DHT11 Sensor:** Monitors temperature and humidity levels.
- **RTC Module:** Ensures timekeeping and enables scheduled tasks.
- **LCD Display:** Provides a user-friendly interface for displaying information.
- **LED Lights:** Offer visual cues about the system's operation, with each color representing a different state or condition.
- **Buttons:** Allow manual intervention and adjustment of settings.
- **Fan:** Moves air through the cooling system and promotes evaporation.
- **Water Level Sensor:** Monitors the water level in the swamp cooler.
- **Vent (Stepper Motor):** Adjusts the position of the vent to customize airflow direction and intensity.

#### States
- **Disabled:** Yellow LED indicates no monitoring of temperature or water levels.
- **Idle:** Green LED indicates monitoring of temperature and water levels.
- **Error:** Red LED indicates low water levels and prompts an error message.
- **Running:** Blue LED indicates the fan is active and cooling the system.
  
#### Cooler Circuit Image
![5B36820F-0D8A-4047-9570-D2AD067656CE](https://github.com/HannahHaggerty/final/assets/113158997/afcdaacb-a6e5-4763-bfc8-d98e18a4f918)

#### Schematic Diagram
![finalSchematic](https://github.com/HannahHaggerty/final/assets/113158997/b288a565-bb4e-4dae-b789-4a9a768fa23a)

