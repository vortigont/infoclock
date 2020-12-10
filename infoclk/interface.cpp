#include "main.h"

#include <EmbUI.h>
#include "interface.h"

// статический класс с готовыми формами для базовых системных натсроек
#include "basicui.h"


//uint8_t lang = LANG::RU;   // default language for text resources

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
    embui.var_create(FPSTR(V_WAPI_CITY_NAME), "");          // Короткое имя города для дисплея
    embui.var_create(FPSTR(V_W_UPD_TIME), TOSTRING(WAPI_DEFAULT_UPDATE_TIME));	            // weather update, hours
    embui.var_create(FPSTR(V_W_UPD_RTR),  TOSTRING(WAPI_DEFAULT_RETRY_TIME));	            // weather update, minutes

    /**
     * обработчики действий
     */ 
    // вывод WebUI секций
    embui.section_handle_add(FPSTR(B_CLOCK), block_page_clock);             // generate "clock" page
    embui.section_handle_add(FPSTR(B_WEATHER), block_page_weather);         // generate "weather" page

   /**
    * регистрируем статические секции для web-интерфейса с системными настройками,
    */
    BasicUI::add_sections();


    // активности
    embui.section_handle_add("brt", ui_set_brightness);                     // регулятор яркости
    embui.section_handle_add(FPSTR(A_UPD_WEATHER), upd_weather);            // обновить погоду
    embui.section_handle_add(FPSTR(A_SET_WEATHER), set_weather);            // save weather settings
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
    embui.autoSaveReset(); // автосохранение конфига будет отсчитываться от этого момента
    interf->json_section_menu();    // открываем секцию "меню"

    interf->option(FPSTR(B_CLOCK),   FPSTR(C_DICT[lang][CD::Clock]));           // пункт меню "часы"
    interf->option(FPSTR(B_WEATHER), FPSTR(C_DICT[lang][CD::Weather]));       // пункт меню "погода"

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

    interf->range("brt", brightness_calc(), 0, 15, 1,"яркость", true);
    interf->button_submit(F("mxreset"), FPSTR(C_DICT[lang][CD::MX_Reset]), FPSTR(T_GRAY));

    interf->json_section_end(); // end of main
    interf->json_frame_flush();     // flush frame
}

void block_page_weather(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->json_frame_interface("");       // саму секцию целиком не обрабатываем

    interf->json_section_main(FPSTR(B_WEATHER), FPSTR(C_DICT[lang][CD::Weather]));

    // Force weather-update button
    interf->button_submit(FPSTR(A_UPD_WEATHER), FPSTR(C_DICT[lang][CD::UPD_Weath]), FPSTR(T_GRAY));

    // форма настроек "Погоды"
    interf->json_section_hidden(FPSTR(A_SET_WEATHER), FPSTR(C_DICT[lang][CD::SET_Weath]));  // секция с заголовком

    interf->spacer(FPSTR(C_DICT[lang][CD::WthNote]));       // Weather setup Note

    interf->text(FPSTR(V_WAPI_KEY), FPSTR(C_DICT[lang][CD::WAPIKEY]));      // weather API-key
    interf->text(FPSTR(V_WAPI_CITY_ID), F("City-id"));                      // city-id for weather API
    interf->text(FPSTR(V_WAPI_CITY_NAME), F("City short name"));            // city-id for weather API

    interf->text(FPSTR(V_W_UPD_TIME), F("weather update, hours"));          // weather update, hours
    interf->text(FPSTR(V_W_UPD_RTR), F(" weather update, min"));            // weather update, minutes

    interf->button_submit(FPSTR(A_SET_WEATHER), FPSTR(T_DICT[lang][TD::D_SAVE]), FPSTR(T_GRAY));
    interf->json_section_end();

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
    refreshWeather();
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
 
    refreshWeather();

    section_main_frame(interf, data);
}


/**
 * регулировка яркости матрицы
 */
void ui_set_brightness(Interface *interf, JsonObject *data){
    if (!interf) return;

    matrix.setIntensity((*data)["brt"]);
}

/**
 * реинициализации матрицы,
 * сбрасывает состояние всех регистров, помогает устранить артефакты на модулях
 */
void ui_mx_reset(Interface *interf, JsonObject *data){
    matrix.reset();
}