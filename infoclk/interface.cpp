#include "main.h"

#include <EmbUI.h>
#include "interface.h"
#include "infoclock.h"
#include "ui_i18n.h"    // localized GUI text-strings

// статический класс с готовыми формами для базовых системных натсроек
#include "basicui.h"

#define CS_MIN 0
#define CS_MAX 16

//uint8_t lang = LANG::RU;   // default language for text resources

extern Infoclock informer;

/**
 * Define configuration variables and controls handlers
 * variables has literal names and are kept within json-configuration file on flash
 * 
 * Control handlers are bound by literal name with a particular method. This method is invoked
 * by manipulating controls
 * 
 */
void create_parameters(){
    LOG(println, F("UI: Creating application vars"));

    /**
     * регистрируем свои переменные
     */
    embui.var_create(FPSTR(V_W_UPD_TIME), WEATHER_UPD_PERIOD);    // weather update, hours
    embui.var_create(FPSTR(V_W_UPD_RTR),  WEATHER_UPD_RETRY);	  // weather update retry, minutes
    embui.var_create(FPSTR(V_MX_W),  MX_DEFAULT_W);
    embui.var_create(FPSTR(V_MX_H),  MX_DEFAULT_H);
    embui.var_create(FPSTR(V_CSPIN), DEFAULT_CS_PIN);             // SPI CS pin

    /**
     * обработчики действий
     */ 
    // вывод WebUI секций
    embui.section_handle_add(FPSTR(B_CLOCK), block_page_clock);             // generate "clock" page
    embui.section_handle_add(FPSTR(B_WEATHER), block_page_weather);         // generate "weather setup" page
    embui.section_handle_add(FPSTR(B_MATRIX), block_page_matrix);           // generate "matrix setup" page
    embui.section_handle_add(FPSTR(B_SENSORS), block_page_sensors);         // generate "sensors" page


   /**
    * регистрируем статические секции для web-интерфейса с системными настройками,
    */
    basicui::add_sections();


    // активности
    embui.section_handle_add("brt", ui_set_brightness);                     // регулятор яркости
    embui.section_handle_add(FPSTR(A_UPD_WEATHER), upd_weather);            // обновить погоду
    embui.section_handle_add(FPSTR(A_SET_WEATHER), set_weather);            // save weather settings
    embui.section_handle_add(FPSTR(A_SET_MATRIX),  set_matrix);             // save matrix settings
    embui.section_handle_add("mxreset", ui_mx_reset);                       // сброс матрицы
}


/**
 * Headlile section
 * this is an overriden weak method that builds our WebUI from the top
 * ==
 * Головная секция
 * переопределенный метод фреймфорка, который начинает строить корень нашего WebUI
 * 
 */
void section_main_frame(Interface *interf, JsonObject *data){
    if (!interf) return;

    interf->json_frame_interface(FPSTR(T_HEADLINE));  // HEADLINE for WebUI
    block_menu(interf, data);                       // Строим UI блок с меню выбора других секций
    interf->json_frame_flush();

    if(!embui.sysData.wifi_sta){                      // если контроллер не подключен к внешней AP, сразу открываем вкладку с настройками WiFi
        LOG(println, F("UI: Opening network setup section"));
        basicui::block_settings_netw(interf, data);
    } else {
        block_page_clock(interf, data);               // Строим основной блок часов 
    }

  interf->json_frame_flush();                         // Close interface section

  // Publish firmware version (visible under menu section)
  interf->json_frame_value();
  interf->value(F("fwver"), F(INFOCLOCK_VERSION_STRING), true);
  interf->json_frame_flush();
}

/**
 * This code builds UI section with menu block on the left
 * 
 */
void block_menu(Interface *interf, JsonObject *data){
    if (!interf) return;
    // создаем меню
    interf->json_section_menu();    // открываем секцию "меню"

    interf->option(FPSTR(B_CLOCK),    FPSTR(C_DICT[lang][CD::Clock]));           // пункт меню "часы"
    interf->option(FPSTR(B_SENSORS),  FPSTR(C_DICT[lang][CD::snsrs]));         // пункт меню "sensors"
    interf->option(FPSTR(B_WEATHER),  FPSTR(C_DICT[lang][CD::Weather]));         // пункт меню "погода"
    interf->option(FPSTR(B_MATRIX),   F("Matrix"));           // пункт меню "матрица"


    /**
     * добавляем в меню пункт - настройки,
     */
    basicui::opt_setup(interf, data);       // пункт меню "настройки"

    interf->json_section_end();
}

/**
 * This code builds UI section with clock settings
 * 
 */
void block_page_clock(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->json_frame_interface();

    interf->json_section_main(FPSTR(B_CLOCK), FPSTR(C_DICT[lang][CD::Clock]));

    interf->range("brt", informer.brightness(), 0, 15, 1,"яркость", true);
    interf->button(F("mxreset"), FPSTR(C_DICT[lang][CD::MX_Reset]), FPSTR(T_GRAY));

    // Simple Clock display
    String clk; embui.timeProcessor.getDateTimeString(clk);
    interf->display(F("clk"), clk);    // Clock display


    interf->json_frame_flush();     // flush frame
}


/**
 * Weather options setup
 * 
 */
void block_page_weather(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->json_frame_interface();

    interf->json_section_main(FPSTR(A_SET_WEATHER), FPSTR(C_DICT[lang][CD::Weather]));

    interf->comment(informer.weatherdata());       // Current Weather value

    // Force weather-update button
    interf->button(FPSTR(A_UPD_WEATHER), FPSTR(C_DICT[lang][CD::UPD_Weath]), FPSTR(T_GRAY));

    interf->spacer(FPSTR(C_DICT[lang][CD::WthNote]));       // Weather setup Note

    interf->text(FPSTR(V_WAPI_KEY), FPSTR(C_DICT[lang][CD::WAPIKEY]));              // weather API-key
    interf->text(FPSTR(V_WAPI_CITY_ID), FPSTR(C_DICT[lang][CD::WthCID]));           // city-id for weather API
    interf->text(FPSTR(V_WAPI_CITY_NAME), FPSTR(C_DICT[lang][CD::WthSrtName]));     // city short name

    interf->number(FPSTR(V_W_UPD_TIME), F("интервал обновления, ч."));          // weather update, hours
    interf->number(FPSTR(V_W_UPD_RTR), F("повтор при ошибке, мин."));            // weather update, minutes

    interf->button_submit(FPSTR(A_SET_WEATHER), FPSTR(T_DICT[lang][TD::D_SAVE]), FPSTR(T_GRAY));

    interf->json_frame_flush();     // flush frame
}

/**
 * Matrix options setup
 * 
 */
void block_page_matrix(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->json_frame_interface();

    interf->json_section_main(FPSTR(B_MATRIX), FPSTR(C_DICT[lang][CD::mtx]));

    interf->range(F("brt"), informer.brightness(), 0, 15, 1, F("яркость"), true);

    interf->number(FPSTR(V_CSPIN), F("SPI CS pin"), 1, CS_MIN, CS_MAX);
    interf->comment(F("Changing CS pin requires MCU reboot"));
    interf->spacer();

    interf->json_section_line(FPSTR(A_SET_MATRIX));

    interf->number(FPSTR(V_MX_W), FPSTR(F("Panel Width")));        // Num of modules W
    interf->number(FPSTR(V_MX_H), FPSTR(F("Panel Height")));       // Num of modules H


    /*
	 * Define if and how the displays are rotated. The first display
	 * (0) is the one closest to the Arduino. rotation can be:
	 *   0: no rotation
	 *   1: 90 degrees clockwise
	 *   2: 180 degrees
	 *   3: 90 degrees counter clockwise
	 */
    interf->select(FPSTR(V_MX_MR), F("MAX Module rotation"));
    interf->option(1, F("No rotation"));
    interf->option(2, F("90 CW"));
    interf->option(3, F("180 CW"));
    interf->option(4, F("90 CCW"));
    interf->json_section_end();     // end of select

    interf->json_section_end();     // end of line

    interf->json_section_line(FPSTR(A_SET_MATRIX));
    interf->checkbox(FPSTR(V_MX_OS), F("Serpentine"), false);	// Serpentine/Zig-zag sequence
    interf->checkbox(FPSTR(V_MX_OV), F("Vertical"), false);	// Vertical sequence
    interf->checkbox(FPSTR(V_MX_VF), F("V-Flip"), false);	// Flip verticaly
    interf->checkbox(FPSTR(V_MX_HF), F("H-Flip"), false);	// Flip horizontaly
    interf->json_section_end();     // end of line

    interf->button_submit(FPSTR(A_SET_MATRIX), FPSTR(T_DICT[lang][TD::D_SAVE]), "blue");

    interf->button(F("mxreset"), FPSTR(C_DICT[lang][CD::MX_Reset]), FPSTR(T_GRAY));

    interf->json_frame_flush();     // flush frame
}


/**
 * This code builds UI section with sensors settings
 * 
 */
void block_page_sensors(Interface *interf, JsonObject *data){

    // if it was a form post, than update settings
    if (data){
        SETPARAM_NONULL(FPSTR(V_SN_TCOMP), informer.clksensor.tempoffset(embui.paramVariant(FPSTR(V_SN_TCOMP))));
        SETPARAM(FPSTR(V_SN_UPD_RATE), informer.snsupdrate(embui.paramVariant(FPSTR(V_W_UPD_TIME))));
    }

    if (!interf) return;
    interf->json_frame_interface();

    interf->json_section_main(FPSTR(B_SENSORS), FPSTR(C_DICT[lang][CD::snsrs]));

    interf->range(FPSTR(V_SN_UPD_RATE), 5, 5, 30, 5, F("Sensors update rate"), false);

    interf->number(FPSTR(V_SN_TCOMP), F("Temp sensor compensation"), 0.1, -5.0, 5.0);

    interf->button_submit(FPSTR(B_SENSORS), FPSTR(T_DICT[lang][TD::D_SAVE]));

    interf->json_frame_flush();     // flush frame
}



/**
 * обработчик статуса (периодического опроса контроллера веб-приложением)
 */
void pubCallback(Interface *interf){
    basicui::embuistatus(interf);
}


// Callback ACTIONS

/**
 * обновить погоду
 */
void upd_weather(Interface *interf, JsonObject *data){
    informer.refreshWeather();
}

/**
 *  Apply weather-related options 
 */
void set_weather(Interface *interf, JsonObject *data){
    if (!data) return;

    // сохраняем параметры настроек погоды
    SETPARAM_NONULL(FPSTR(V_WAPI_KEY));
    SETPARAM_NONULL(FPSTR(V_WAPI_CITY_ID));
    SETPARAM_NONULL(FPSTR(V_WAPI_CITY_NAME));
    SETPARAM(FPSTR(V_W_UPD_TIME));
    SETPARAM(FPSTR(V_W_UPD_RTR));
 
    informer.refreshWeather();

    if(interf)
        section_main_frame(interf, nullptr);
}

/**
 *  Apply matrix-related options 
 */
void set_matrix(Interface *interf, JsonObject *data){
    if (!data) return;

    // save cs pin to cfg
    uint8_t cs = (*data)[FPSTR(V_CSPIN)].as<unsigned short>();
    if (cs >= CS_MIN && cs <= CS_MAX && cs < 6 && cs >11){
        SETPARAM(FPSTR(V_CSPIN));
    }

    SETPARAM(FPSTR(V_MX_W));
    SETPARAM(FPSTR(V_MX_H));
    SETPARAM_NONULL(FPSTR(V_MX_VF));
    SETPARAM_NONULL(FPSTR(V_MX_HF));
    SETPARAM_NONULL(FPSTR(V_MX_OS));
    SETPARAM_NONULL(FPSTR(V_MX_OV));
    SETPARAM_NONULL(FPSTR(V_MX_MR));

    informer.mxPaneSetup(
        (*data)[FPSTR(V_MX_OS)].as<unsigned short>(), (*data)[FPSTR(V_MX_OV)].as<unsigned short>(),
        (*data)[FPSTR(V_MX_VF)].as<unsigned short>(), (*data)[FPSTR(V_MX_HF)].as<unsigned short>(),
        (*data)[FPSTR(V_MX_MR)].as<unsigned short>()
    );

    if(interf) block_page_matrix(interf, nullptr);
}


/**
 * регулировка яркости матрицы
 */
void ui_set_brightness(Interface *interf, JsonObject *data){
    if (!data) return;
    informer.brightness((*data)["brt"]);
}

/**
 * реинициализации матрицы,
 * сбрасывает состояние всех регистров, помогает устранить артефакты на модулях
 */
void ui_mx_reset(Interface *interf, JsonObject *data){
    informer.mxreset();
}