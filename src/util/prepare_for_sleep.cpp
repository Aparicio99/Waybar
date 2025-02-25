#include "util/prepare_for_sleep.h"

#include <gio/gio.h>

namespace {
class PrepareForSleep {
 private:
  PrepareForSleep() {
    GError *error = NULL;
    login1_connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (!login1_connection) {
      throw std::runtime_error("Unable to connect to the SYSTEM Bus!...");
    } else {
      login1_id = g_dbus_connection_signal_subscribe(
          login1_connection, "org.freedesktop.login1", "org.freedesktop.login1.Manager",
          "PrepareForSleep", "/org/freedesktop/login1", NULL, G_DBUS_SIGNAL_FLAGS_NONE,
          prepareForSleep_cb, this, NULL);
    }
  }

  static void prepareForSleep_cb(GDBusConnection *system_bus, const gchar *sender_name,
                                 const gchar *object_path, const gchar *interface_name,
                                 const gchar *signal_name, GVariant *parameters,
                                 gpointer user_data) {
    if (g_variant_is_of_type(parameters, G_VARIANT_TYPE("(b)"))) {
      gboolean sleeping;
      g_variant_get(parameters, "(b)", &sleeping);

      PrepareForSleep *self = static_cast<PrepareForSleep *>(user_data);
      self->signal.emit(sleeping);
    }
  }

 public:
  static PrepareForSleep &GetInstance() {
    static PrepareForSleep instance;
    return instance;
  }
  waybar::SafeSignal<bool> signal;

 private:
  guint login1_id;
  GDBusConnection *login1_connection;
};
}  // namespace

waybar::SafeSignal<bool> &waybar::util::prepare_for_sleep() {
  return PrepareForSleep::GetInstance().signal;
}
