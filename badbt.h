#include <notification/notification_messages.h>
#include <bt/bt_service/bt.h>
#include <gui/gui.h>

typedef struct {
    Bt* bt;
    Gui* gui;
    NotificationApp* notifications;
    bool connected;
} BtHid;