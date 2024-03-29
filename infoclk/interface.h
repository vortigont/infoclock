#pragma once

// Interface blocks
void block_menu(Interface *interf, JsonObject *data);
void block_page_clock(Interface *interf, JsonObject *data);
void block_page_weather(Interface *interf, JsonObject *data);
void block_page_matrix(Interface *interf, JsonObject *data);
void block_page_sensors(Interface *interf, JsonObject *data);

//void remote_action(RA action, ...);
//void uploadProgress(size_t len, size_t total);

//  ACTIONS
void action_demopage(Interface *interf, JsonObject *data);
void upd_weather(Interface *interf, JsonObject *data);
void set_weather(Interface *interf, JsonObject *data);
void set_matrix(Interface *interf, JsonObject *data);
void ui_set_brightness(Interface *interf, JsonObject *data);
void ui_mx_reset(Interface *interf, JsonObject *data);

// Callbacks
void pubCallback(Interface *interf);
