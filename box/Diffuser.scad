// WS2812 / WS2812b Led Matrix Diffuser by malebuffy

PCB_HEIGHT = 85; // Size of PCB in mm
PCB_WIDTH = 85; // Size of PCB in mm
NUMBER_LEDS_HORIZONTAL=8; // Number of Leds in a row
NUMBER_LEDS_VERTICAL=8;   // Number of Leds in a column
DIFFUSER_HEIGHT = 4; // Height of diffuser
LED_SIZE = 5; // Size of leds in mm
DISTANCE_BETWEEN_LEDS=5; // Distance between leds in mm
WALL_THINKESS=1.2; // Thinkess of walls
PCB_BORDER = 3;  // PCB Outer Boarder in mm

buffer=DISTANCE_BETWEEN_LEDS-WALL_THINKESS;


difference() {

cube([PCB_HEIGHT,PCB_WIDTH,DIFFUSER_HEIGHT]); // Create Main PCB Diffuser

for ( i = [0:NUMBER_LEDS_HORIZONTAL-1] ) {
    
    translate([PCB_BORDER,(LED_SIZE+DISTANCE_BETWEEN_LEDS)*i,-1])
    
    for ( j = [0:NUMBER_LEDS_VERTICAL-1] ) {
        
     translate([(LED_SIZE+DISTANCE_BETWEEN_LEDS)*j,PCB_BORDER,-1]) cube([LED_SIZE+buffer,LED_SIZE+buffer,DIFFUSER_HEIGHT+5]);   
        
        
        
  }
 }
}