/*
 * This file is part of Infoclock project
 * a controller for dotmatrix display used as a home informer panel
 * in some of my other automation projects
 *
 *  © 2019 Emil Muratov (vortigont)
 */

#include "main.h"
// Libs
#include <Adafruit_GFX.h>	// need to override bundled "glcdfont.c" font with cyr version
#include <Max72xxPanel.h>
#include <ESP8266HTTPClient.h>
#include "sensors.h"

#define DEFAULT_DIM_X   8       // default panel dimensions, pixels x
#define DEFAULT_DIM_Y   8       // default panel dimensions, pixels y

#define MX_DEFAULT_W    4   // Matrix WIDTH (number of 8x8 MAX modules)
#define MX_DEFAULT_H    4   // Matrix HEIGHT (number of 8x8 MAX modules)
#define MX_DEFAULT_VF   0   // Canvas Vertical flip
#define MX_DEFAULT_HF   0   // Canvas Horizontal flip
#define MX_DEFAULT_OV   0   // Modules order - Vertical/Horizontal
#define MX_DEFAULT_OS   0   // Modules order - Serpentine/Zig-Zag
#define MX_DEFAULT_MR   0   // Modules rotation (90 degree turns)

class Infoclock {

public:
    /**
     * constructor declaraion
     */
    Infoclock();

    /** @brief init - initialize Informer object, create dynamic valuies and objects
     * 
     *  @param _x - display panel dimensions, absolute width
     *  @param _y - display panel dimensions, absolute height
     */
    void init(const int16_t _x, const int16_t _y);

    /** @brief onNetIfUp - коллбек для внешнего события "сеть доступна"
     * 
     */
    void onNetIfUp();

    /** @brief onNetIfDown - коллбек для внешнего события "сеть НЕ доступна"
     * 
     */
    void onNetIfDown();

    // Display manipulation functions

    // Task Callback methods prototypes
    void refreshWeather();  // restart weather timer
    void mxPaneSetup(const bool serp,  const bool vert, const bool vflip, const bool hflip, const int mr=0);

    // Get current display brightness
    uint8_t brightness(){ return brightness_calc();};
    // Set display brightness, returns resulting brightness
    void brightness(uint8_t b);

    const String& weatherdata(){return weather;};

    Sensors clksensor;    // sensors object


private:
    std::unique_ptr<Max72xxPanel> matrix = nullptr;   // указатель для объкета матрицы, будет инициализирован позже

    //static time_t now;
    uint8_t lastmin = 0;
    bool wscroll = 1;	 // do weather scroll
    uint16_t w, h;       // matrix width, height

    // scroll y pointers
    int strp1, strp2 = 0;

    // String for current weather info
    String weather;

    // String for weather display scroller
    String tape;

    // Sensors
    String sensorstr;


    // Tasks
    Task tWeatherUpd;
    Task tSensorUpd;
    Task tScroller;
    Task tSecondsPulse;
    Task tDrawTicks;

    void doSeconds();		// every second pulse task (tSecondsPulse)
    void bigClk ();			// Draw clock with big font
    void sectick(uint16_t x, uint16_t y);	// Draw seconds ticks
    bool drawticks();		// Draw hh:mm ticks
    void clearticks();		// clear hh:mm ticks
    void updsensstr();		// Update data from sensors and build text string
    void panescroller();		// Scroll text over pane

    void GetWeather();		// Update weather info with HTTP client
    void parseWeather(String& s);		// Parse response from weather server

    // set panel dimensions
    void setDimensions(const int16_t _x, const int16_t _y){
        // I do not need negatives here
        if (_x && _y){
            w = _x; h = _y;
        }
    }

    uint8_t brightness_calc(void);		// calculate display brightness for current time of day

    template <typename T> void mtxprint( const T& str, uint16_t x, uint16_t y);
    template <typename T> void scroll( const T& str, int y, int& scrollptr);

    // recode UTF8 rus strings
    static bool utf8toCP1251(const String& source, String& dst, bool concat = false);

}; // end of Infoclock class