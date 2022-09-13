// ************************* check_AQ.h **********************************************
#define INIT_AQ 0
#define AQ_CHECK 1
#define NO_AQ_CHECK 2
#define AQ_TIME 580
#define AQ_UP_TIME 5000
#define AQ_JUMP_TIME 100
#define AIR_POLLUTE_LEVEL0 20
#define AIR_SATURTION 200
#define AIR_POLLUTE_PASS 5
#define AQ_CLEAN_STATE 0
#define AQ_POL_LEV1 1
#define AQ_POL_LEV2 2
#define AQ_POL_LEV3 3
#define AQ_POL_LEV4 4

#define CALIBRATION_DEFAULT 500
#define CALIBRATION_OFFSET 500
#define	CHANNEL_AQ       2
// --- function prototypes ---------
void init_AQ_calibration(void);
void check_Air_Quality();


// --- variables --------- 
extern unsigned short int  aq_calibration;
extern unsigned short int aq_level_value[4];
extern unsigned short int aq_value_temp;
extern unsigned char  run_check_aq;
extern unsigned char air_cal_point[4];//= {30,55,100,120,150,200,100};	// three alarm trigger points//ST

//extern unsigned char  running_average_counter = 0;
//unsigned short int  current_average;
//unsigned short int  running_average;
//unsigned short int  air_slope_tolerance;
extern unsigned short int  show_aq_sen;
extern unsigned short int  aq_value;

// --- usefull external function prototypes ---------
extern void delay_us(unsigned int nus);

// --- usefull external variables ---------
extern struct link  b;
//extern unsigned int xdata analog_in_buffer[2];
extern  short int temperature ; 
// --- EEPROM ----------------------------------
extern unsigned char eeprom_read_byte(unsigned short int addr);
extern void write_eeprom(unsigned short int addr, unsigned char ch);
//extern void write_eeprom(unsigned short int addr, unsigned char ch);
extern unsigned char alarm_delay;