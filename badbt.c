#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

typedef struct {
    int x;
    int y;
    int move_x, move_y;
} HelloWorldModel;

typedef struct {
    HelloWorldModel* model;
    FuriMutex* model_mutex;

    FuriMessageQueue* event_queue;

    ViewPort* view_port;
    Gui* gui;
} HelloWorld;

void draw_callback(Canvas* canvas, void* ctx) {
    HelloWorld* hello_world = ctx;
    furi_check(furi_mutex_acquire(hello_world->model_mutex, FuriWaitForever) == FuriStatusOk);

    //canvas_draw_box(canvas, hello_world->model->x, hello_world->model->y, 4, 4);
    canvas_draw_frame(canvas, 0, 0, 128, 64);
    canvas_draw_str(canvas, hello_world->model->x, hello_world->model->y, "Hello World!");

    furi_mutex_release(hello_world->model_mutex);
}

void input_callback(InputEvent* input, void* ctx) {
    HelloWorld* hello_world = ctx;
    // Puts input onto event queue with priority 0, and waits until completion.
    furi_message_queue_put(hello_world->event_queue, input, FuriWaitForever);
}

HelloWorld* hello_world_alloc() {
    HelloWorld* instance = malloc(sizeof(HelloWorld));

    instance->model = malloc(sizeof(HelloWorldModel));
    instance->model->x = rand() % 128;
    instance->model->y = rand() % 64;
    instance->model->move_x = 2;
    instance->model->move_y = 2;

    instance->model_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    instance->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    instance->view_port = view_port_alloc();
    view_port_draw_callback_set(instance->view_port, draw_callback, instance);
    view_port_input_callback_set(instance->view_port, input_callback, instance);

    instance->gui = furi_record_open("gui");
    gui_add_view_port(instance->gui, instance->view_port, GuiLayerFullscreen);

    return instance;
}

void hello_world_free(HelloWorld* instance) {
    view_port_enabled_set(instance->view_port, false); // Disabsles our ViewPort
    gui_remove_view_port(instance->gui, instance->view_port); // Removes our ViewPort from the Gui
    furi_record_close("gui"); // Closes the gui record
    view_port_free(instance->view_port); // Frees memory allocated by view_port_alloc
    furi_message_queue_free(instance->event_queue);

    furi_mutex_free(instance->model_mutex);

    free(instance->model);
    free(instance);
}

int32_t hello_world_app(void* p) {
    UNUSED(p);

    HelloWorld* hello_world = hello_world_alloc();

    InputEvent event;
    for(bool processing = true; processing;) {
        // Pops a message off the queue and stores it in `event`.
        // No message priority denoted by NULL, and 100 ticks of timeout.
        FuriStatus status = furi_message_queue_get(hello_world->event_queue, &event, 100);
        furi_check(furi_mutex_acquire(hello_world->model_mutex, FuriWaitForever) == FuriStatusOk);
        if(status == FuriStatusOk) {
            if(event.type == InputTypePress) {
                switch(event.key) {
                case InputKeyUp:
                    hello_world->model->move_y -= 2;
                    break;
                case InputKeyDown:
                    hello_world->model->move_y += 2;
                    break;
                case InputKeyLeft:
                    hello_world->model->move_x -= 2;
                    break;
                case InputKeyRight:
                    hello_world->model->move_x += 2;
                    break;
                case InputKeyOk:
                case InputKeyBack:
                    processing = false;
                    break;
                }
            }
        }

        if(((hello_world->model->x + hello_world->model->move_x) > 127) ||
           ((hello_world->model->x + hello_world->model->move_x) < 1)) {
            hello_world->model->move_x = (hello_world->model->move_x) * -1;
        }
        hello_world->model->x += hello_world->model->move_x;

        if(((hello_world->model->y + hello_world->model->move_y) > 63) ||
           ((hello_world->model->y + hello_world->model->move_y) < 1)) {
            hello_world->model->move_y = (hello_world->model->move_y) * -1;
        }
        hello_world->model->y += hello_world->model->move_y;
        furi_mutex_release(hello_world->model_mutex);
        view_port_update(hello_world->view_port); // signals our draw callback
    }
    hello_world_free(hello_world);
    return 0;
}