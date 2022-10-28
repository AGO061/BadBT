#include <input/input.h>
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_bt_hid.h>
#include "badbt.h"

uint8_t screen=0;

void badbt_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    if (screen==0){
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 10, "BadBT");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 22, "Waiting for connection...");
    } else if (screen==1){
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 10, "BadBT");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 22, "BLUETOOTH CONNECTED!");
    }
}

void badbt_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}


void bt_hid_connection_status_changed_callback(BtStatus status, void* context) {
    furi_assert(context);
    
    BtHid* app=context;
    app->connected = (status == BtStatusConnected);
    if(app->connected) {
        screen=1;
        notification_internal_message(app->notifications, &sequence_set_blue_255);
    } else {
        screen=0;
        notification_internal_message(app->notifications, &sequence_reset_blue);
    }
}






int32_t badbt_app(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, badbt_draw_callback, NULL);
    view_port_input_callback_set(view_port, badbt_input_callback, event_queue);
    BtHid* app = malloc(sizeof(BtHid));

    // Gui
    app->gui = furi_record_open(RECORD_GUI);

    // Bt
    app->bt = furi_record_open(RECORD_BT);

    // Notifications
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    app->connected=false;
    // Register view port in GUI
    gui_add_view_port(app->gui, view_port, GuiLayerFullscreen);
    

    InputEvent event;

    while(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk) {
        if(event.type == InputTypeShort && event.key == InputKeyBack) {
            //Exiting application
            notification_message(app->notifications, &sequence_reset_vibro);
            notification_message(app->notifications, &sequence_reset_green);
            break;
        }
        /*if(event.key == InputKeyOk) {
            if(event.type == InputTypePress) {
                //notification_message(notification, &sequence_set_vibro_on);
                notification_message(app->notifications, &sequence_set_green_255);
                

                //screen sequences


            } else if(event.type == InputTypeRelease) {
                //notification_message(notification, &sequence_reset_vibro);
                notification_message(app->notifications, &sequence_reset_green);
            }
        }*/
        if (screen==0){
            bt_set_status_changed_callback(app->bt, bt_hid_connection_status_changed_callback, app->notifications);
        }
    }

    gui_remove_view_port(app->gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    // Close records
    furi_record_close(RECORD_GUI);
    app->gui = NULL;
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;
    furi_record_close(RECORD_BT);
    app->bt = NULL;
    free(app);
    return 0;
}

