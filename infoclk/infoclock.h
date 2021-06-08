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

#define DEFAULT_DIM_X 8     // default panel dimensions
#define DEFAULT_DIM_Y 8


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

    // Task Callback methods prototypes
    void refreshWeather();  // restart weather timer
    void mxPaneRotation(const bool serp,  const bool vert, const bool vflip, const bool hflip, const int mr=0);

    // Display manipulation functions
    uint8_t brightness_calc(void);		// calculate display brightness for current time of day

    // set panel dimensions
    void setDimensions(const int16_t _x, const int16_t _y){
        // I do not need negatives here
        if (_x && _y){
            w = _x; h = _y;
        }
        // to-do: recreate display object here
    }

private:
    std::unique_ptr<Max72xxPanel> matrix = nullptr;   // указатель для объкета матрицы, будет инициализирован позже

    //static time_t now;
    uint8_t lastmin = 0;
    bool wscroll = 1;	 // do weather scroll
    uint16_t w, h;       // matrix width, height

    // scroll y pointers
    int strp1, strp2 = 0;

    // String for weather info
    String tape;

    // Sensors
    String sensorstr;


    Sensors clksensor;    // sensor object
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
    void GetWeather();		// Update weather info with HTTP client
    void updsensstr();		// Update data from sensors and build text string
    void ParseWeather(String s);		// Parse response from weather server
    void panescroller();		// Scroll text over pane

    // work with data sources
    void readbme280(float& t, float& h, float& p, float& dew);	// retreive data from BME280 sensor
    void readsi7021(float& t, float& h);	                    // retreive data from si7021 sensor

    template <typename T> void mtxprint( const T& str, uint16_t x, uint16_t y);
    template <typename T> void scroll( const T& str, int y, int& scrollptr);

    // recode UTF8 rus strings
    // http://arduino.ru/forum/programmirovanie/rusifikatsiya-biblioteki-adafruit-gfx-i-vyvod-russkikh-bukv-na-displei-v-kodi
    static String utf8toCP1251(String source);
    static String utf8rus(String source);

}; // end of Infoclock class