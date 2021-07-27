#include "main.h"

#include <EmbUI.h>
#include "interface.h"
#include "infoclock.h"

// статический класс с готовыми формами для базовых системных натсроек
#include "basicui.h"

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
    embui.var_create(FPSTR(V_WAPI_KEY), "");                // API key for OpenWeather
    embui.var_create(FPSTR(V_WAPI_CITY_ID), F("519690"));   // Новороссийск - 518255, Санкт-Петербург - "519690"
    embui.var_create(FPSTR(V_WAPI_CITY_NAME), "");          // Короткое имя города для вывода
    embui.var_create(FPSTR(V_W_UPD_TIME), TOSTRING(WEATHER_UPD_PERIOD));    // weather update, hours
    embui.var_create(FPSTR(V_W_UPD_RTR),  TOSTRING(WEATHER_UPD_RETRY));	    // weather update, minutes
    embui.var_create(FPSTR(V_MX_W), TOSTRING(MX_DEFAULT_W));	            // Matrix W
    embui.var_create(FPSTR(V_MX_H), TOSTRING(MX_DEFAULT_H));	            // Matrix H
    embui.var_create(FPSTR(V_MX_MR), TOSTRING(MX_DEFAULT_MR));	            // Matrix Rotation
    embui.var_create(FPSTR(V_MX_VF), TOSTRING(MX_DEFAULT_VF));	            // Matrix Rotation
    embui.var_create(FPSTR(V_MX_HF), TOSTRING(MX_DEFAULT_HF));	            // Matrix Rotation
    embui.var_create(FPSTR(V_MX_OS), TOSTRING(MX_DEFAULT_OV));	            // Matrix Rotation
    embui.var_create(FPSTR(V_MX_OV), TOSTRING(MX_DEFAULT_OR));	            // Matrix Rotation

    //embui.var_create(FPSTR(V_MX_CR), TOSTRING(MX_DEFAULT_CR));	            // Matrix canvas Rotation

    /**
     * обработчики действий
     */ 
    // вывод WebUI секций
    embui.section_handle_add(FPSTR(B_CLOCK), block_page_clock);             // generate "clock" page
    embui.section_handle_add(FPSTR(B_WEATHER), block_page_weather);         // generate "weather" page
    embui.section_handle_add(FPSTR(B_MATRIX), block_page_matrix);         // generate "weather" page


   /**
    * регистрируем статические секции для web-интерфейса с системными настройками,
    */
    BasicUI::add_sections();


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
        BasicUI::block_settings_netw(interf, data);
    } else {
        block_page_clock(interf, data);               // Строим основной блок часов 
    }

}

/**
 * This code builds UI section with menu block on the left
 * 
 */
void block_menu(Interface *interf, JsonObject *data){
    if (!interf) return;
    // создаем меню
    interf->json_section_menu();    // открываем секцию "меню"

    interf->option(FPSTR(B_CLOCK),   FPSTR(C_DICT[lang][CD::Clock]));           // пункт меню "часы"
    interf->option(FPSTR(B_WEATHER), FPSTR(C_DICT[lang][CD::Weather]));         // пункт меню "погода"
    interf->option(FPSTR(B_MATRIX),  F("Matrix"));           // пункт меню "матрица"

    /**
     * добавляем в меню пункт - настройки,
     */
    BasicUI::opt_setup(interf, data);       // пункт меню "настройки"

    interf->json_section_end();
}

/**
 * This code builds UI section with clock settings
 * 
 */
void block_page_clock(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->json_frame_interface("");       // саму секцию целиком не обрабатываем

    interf->json_section_main(FPSTR(B_CLOCK), FPSTR(C_DICT[lang][CD::Clock]));

    interf->range("brt", informer.brightness_calc(), 0, 15, 1,"яркость", true);
    interf->button_submit(F("mxreset"), FPSTR(C_DICT[lang][CD::MX_Reset]), FPSTR(T_GRAY));

    interf->json_section_end(); // end of main
    interf->json_frame_flush();     // flush frame
}


/**
 * Weather options setup
 * 
 */
void block_page_weather(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->json_frame_interface("");       // саму секцию целиком не обрабатываем

    interf->json_section_main(FPSTR(B_WEATHER), FPSTR(C_DICT[lang][CD::Weather]));

    // Force weather-update button
    interf->button_submit(FPSTR(A_UPD_WEATHER), FPSTR(C_DICT[lang][CD::UPD_Weath]), FPSTR(T_GRAY));

    // форма настроек "Погоды"
    interf->json_section_hidden(FPSTR(A_SET_WEATHER), FPSTR(C_DICT[lang][CD::OPT_Weath]));      // секция с заголовком

    interf->spacer(FPSTR(C_DICT[lang][CD::WthNote]));       // Weather setup Note

    interf->text(FPSTR(V_WAPI_KEY), FPSTR(C_DICT[lang][CD::WAPIKEY]));              // weather API-key
    interf->text(FPSTR(V_WAPI_CITY_ID), FPSTR(C_DICT[lang][CD::WthCID]));           // city-id for weather API
    interf->text(FPSTR(V_WAPI_CITY_NAME), FPSTR(C_DICT[lang][CD::WthSrtName]));     // city short name

    interf->text(FPSTR(V_W_UPD_TIME), F("интервал обновления, ч."));          // weather update, hours
    interf->text(FPSTR(V_W_UPD_RTR), F("повтор при ошибке, мин."));            // weather update, minutes

    interf->button_submit(FPSTR(A_SET_WEATHER), FPSTR(T_DICT[lang][TD::D_SAVE]), FPSTR(T_GRAY));
    interf->json_section_end();

    interf->json_section_end();     // end of main
    interf->json_frame_flush();     // flush frame
}

/**
 * Matrix options setup
 * 
 */
void block_page_matrix(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->json_frame_interface("");

    interf->json_section_main(FPSTR(B_MATRIX), FPSTR(C_DICT[lang][CD::Matrix]));

    interf->range(F("brt"), informer.brightness_calc(), 0, 15, 1, F("яркость"), true);

    interf->json_section_line(FPSTR(A_SET_MATRIX));

    interf->text(FPSTR(V_MX_W), FPSTR(F("Panel Width")));        // Num of modules W
    interf->text(FPSTR(V_MX_H), FPSTR(F("Panel Height")));       // Num of modules H


    /*
	 * Define if and how the displays are rotated. The first display
	 * (0) is the one closest to the Arduino. rotation can be:
	 *   0: no rotation
	 *   1: 90 degrees clockwise
	 *   2: 180 degrees
	 *   3: 90 degrees counter clockwise
	 */
    interf->select(FPSTR(V_MX_MR), embui.param(FPSTR(V_MX_MR)), F("MAX Module rotation"), false, false);
    interf->option("1", F("No rotation"));
    interf->option("2", F("90 CW"));
    interf->option("3", F("180 CW"));
    interf->option("4", F("90 CCW"));
    interf->json_section_end();

    interf->json_section_end();     // end of line

    interf->json_section_line(FPSTR(A_SET_MATRIX));

    interf->checkbox(FPSTR(V_MX_OS), embui.param(FPSTR(V_MX_OS)), F("Serpentine"), false);	// Serpentine/Zig-zag sequence
    interf->checkbox(FPSTR(V_MX_OV), embui.param(FPSTR(V_MX_OV)), F("Vertical"), false);	// Vertical sequence
    interf->checkbox(FPSTR(V_MX_VF), embui.param(FPSTR(V_MX_VF)), F("V-Flip"), false);	// Flip verticaly
    interf->checkbox(FPSTR(V_MX_HF), embui.param(FPSTR(V_MX_HF)), F("H-Flip"), false);	// Flip horizontaly
    interf->json_section_end();     // end of line


    interf->button_submit(FPSTR(A_SET_MATRIX), FPSTR(T_DICT[lang][TD::D_SAVE]), "blue");
//FPSTR(T_BLUE)

    // форма настроек "Погоды"
    //interf->json_section_hidden(FPSTR(A_SET_WEATHER), FPSTR(C_DICT[lang][CD::SET_Weath]));  // секция с заголовком

    interf->json_section_end();     // end of main
    interf->json_frame_flush();     // flush frame
}


/**
 * обработчик статуса (периодического опроса контроллера веб-приложением)
 */
void pubCallback(Interface *interf){
    BasicUI::embuistatus(interf);
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
    SETPARAM(FPSTR(V_WAPI_KEY));
    SETPARAM(FPSTR(V_WAPI_CITY_ID));
    SETPARAM(FPSTR(V_WAPI_CITY_NAME));
    SETPARAM(FPSTR(V_W_UPD_TIME));
    SETPARAM(FPSTR(V_W_UPD_RTR));
 
    informer.refreshWeather();

    section_main_frame(interf, data);
}

/**
 *  Apply matrix-related options 
 */
void set_matrix(Interface *interf, JsonObject *data){
    if (!data) return;

    SETPARAM(FPSTR(V_MX_W));
    SETPARAM(FPSTR(V_MX_H));
    SETPARAM(FPSTR(V_MX_VF));
    SETPARAM(FPSTR(V_MX_HF));
    SETPARAM(FPSTR(V_MX_OS));
    SETPARAM(FPSTR(V_MX_OV));
    SETPARAM(FPSTR(V_MX_MR));

    informer.mxPaneRotation(
        (*data)[FPSTR(V_MX_OS)].as<unsigned short>(), (*data)[FPSTR(V_MX_OV)].as<unsigned short>(),
        (*data)[FPSTR(V_MX_VF)].as<unsigned short>(), (*data)[FPSTR(V_MX_HF)].as<unsigned short>(),
        (*data)[FPSTR(V_MX_MR)].as<unsigned short>()
    );

    block_page_matrix(interf, data);
}


/**
 * регулировка яркости матрицы
 */
void ui_set_brightness(Interface *interf, JsonObject *data){
    if (!interf) return;
    //matrix->setIntensity((*data)["brt"]);
}

/**
 * реинициализации матрицы,
 * сбрасывает состояние всех регистров, помогает устранить артефакты на модулях
 */
void ui_mx_reset(Interface *interf, JsonObject *data){
    //matrix->reset();
}