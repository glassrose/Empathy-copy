#include "config.h"
#include "empathy-roster-group.h"
#include "empathy-utils.h"
#include "empathy-ui-utils.h"

#include <libempathy/empathy-connection-aggregator.h>
#include <telepathy-glib/telepathy-glib.h>
#include <glib/gi18n-lib.h>

#define DEBUG_FLAG EMPATHY_DEBUG_CONTACT
#include "empathy-debug.h"

G_DEFINE_TYPE (EmpathyRosterGroup, empathy_roster_group, GTK_TYPE_EXPANDER)

enum
{
  PROP_NAME = 1,
  PROP_ICON,
  N_PROPS
};

/*
enum
{
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];
*/

struct _EmpathyRosterGroupPriv
{
  gchar *name;
  gchar *icon_name;

  /* Widgets associated with this group. EmpathyRosterGroup is not responsible
   * of packing/displaying these widgets. This hash table is a just a set
   * to keep track of them. */
  GHashTable *widgets;
};

static gboolean label_button_press_event_cb (GtkWidget *label_event_box,
    GdkEventButton *event,
    gpointer user_data);

static void
roster_group_set_label (EmpathyRosterGroup *self,
    GtkWidget *label_widget)
{
  gchar *tmp;
  GtkWidget *label, *label_event_box;

  tmp = g_strdup_printf ("<b>%s</b>", self->priv->name);
  label = gtk_label_new (tmp);
  g_free (tmp);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

  label_event_box = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (label_event_box), label);
  gtk_event_box_set_above_child (GTK_EVENT_BOX (label_event_box), TRUE);
  gtk_widget_add_events (label_event_box, GDK_BUTTON_PRESS_MASK);
  g_signal_connect (label_event_box, "button-press-event",
      G_CALLBACK (label_button_press_event_cb), self);

  gtk_box_pack_start (GTK_BOX (label_widget), label_event_box, TRUE, TRUE, 0);

  gtk_widget_show_all (label_widget);
}

static void
label_edited_cb (GtkWidget *entry,
    EmpathyRosterGroup *roster_group)
{
  gchar *new_label = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
  GtkWidget *label_widget = gtk_expander_get_label_widget (
      GTK_EXPANDER (roster_group));

  gtk_container_remove (GTK_CONTAINER (label_widget), entry);

  if (!EMP_STR_EMPTY (new_label) &&
      tp_strdiff (roster_group->priv->name, new_label))
  {
    EmpathyConnectionAggregator *aggregator;

    DEBUG ("rename group '%s' to '%s'", roster_group->priv->name, new_label);

    /* Set new group name */
    aggregator = empathy_connection_aggregator_dup_singleton ();
    empathy_connection_aggregator_rename_group (aggregator,
        roster_group->priv->name, new_label);
    g_object_unref (aggregator);

    roster_group->priv->name = new_label;
  }
  roster_group_set_label (roster_group, label_widget);

  g_free (new_label);
}

static void
roster_group_edit_selected_cb (GtkMenuItem *menuitem,
    gpointer user_data)
{
  EmpathyRosterGroup *group = EMPATHY_ROSTER_GROUP (user_data);
  GtkWidget *label_widget = gtk_expander_get_label_widget (
      GTK_EXPANDER (group));
  GList *children = gtk_container_get_children (GTK_CONTAINER (label_widget));
  GtkWidget *label = GTK_WIDGET ((g_list_last (children))->data);
  GtkWidget *entry = gtk_entry_new ();

  gtk_container_remove (GTK_CONTAINER (label_widget), label);

  gtk_entry_set_text (GTK_ENTRY (entry), group->priv->name);
  g_signal_connect (entry, "activate", G_CALLBACK (label_edited_cb), group);
  gtk_box_pack_start (GTK_BOX (label_widget), entry, TRUE, TRUE, 0);
  gtk_widget_show_all (label_widget);

  g_list_free (children);
}

static gboolean
label_button_press_event_cb (GtkWidget *label_event_box,
    GdkEventButton *event,
    gpointer user_data)
{
  GtkWidget *menu, *item, *label;

  if (event->type != GDK_BUTTON_PRESS)
    return TRUE;

  if (event->button != 3)
    return TRUE;

  label = (GtkWidget *) (gtk_container_get_children (
      GTK_CONTAINER (label_event_box)))->data;
  menu = empathy_context_menu_new (GTK_WIDGET (label));

  item = gtk_menu_item_new_with_mnemonic (_("_Edit"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  tp_g_signal_connect_object (item, "activate",
      G_CALLBACK (roster_group_edit_selected_cb), user_data, 0);

  gtk_widget_show (item);
  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
      event->button, event->time);

  return FALSE;
}

static void
empathy_roster_group_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  EmpathyRosterGroup *self = EMPATHY_ROSTER_GROUP (object);

  switch (property_id)
    {
      case PROP_NAME:
        g_value_set_string (value, self->priv->name);
        break;
      case PROP_ICON:
        g_value_set_string (value, self->priv->icon_name);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
empathy_roster_group_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  EmpathyRosterGroup *self = EMPATHY_ROSTER_GROUP (object);

  switch (property_id)
    {
      case PROP_NAME:
        g_assert (self->priv->icon_name == NULL); /* construct */
        self->priv->name = g_value_dup_string (value);
        break;
      case PROP_ICON:
        g_assert (self->priv->icon_name == NULL); /* construct-only */
        self->priv->icon_name = g_value_dup_string (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
hack_remap_label_widget (EmpathyRosterGroup *self,
    gpointer user_data)
{
  GtkWidget *label_widget = gtk_expander_get_label_widget (GTK_EXPANDER (self));

  if (label_widget)
    {
      gtk_widget_unmap (label_widget);
      gtk_widget_map (label_widget);
    }
}

static void
empathy_roster_group_constructed (GObject *object)
{
  EmpathyRosterGroup *self = EMPATHY_ROSTER_GROUP (object);
  void (*chain_up) (GObject *) =
      ((GObjectClass *) empathy_roster_group_parent_class)->constructed;
  GtkWidget *box;

  if (chain_up != NULL)
    chain_up (object);

  g_assert (self->priv->name != NULL);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

  /* Icon, if any */
  if (!tp_str_empty (self->priv->icon_name))
    {
      GtkWidget *icon;

      icon = gtk_image_new_from_icon_name (self->priv->icon_name,
          GTK_ICON_SIZE_MENU);

      if (icon != NULL)
        gtk_box_pack_start (GTK_BOX (box), icon, FALSE, FALSE, 0);
    }

  /* Label */
  roster_group_set_label (self, box);

  gtk_expander_set_label_widget (GTK_EXPANDER (self), box);

  /* This hack should not have been needed.
     See https://bugzilla.gnome.org/show_bug.cgi?id=705971 */
  g_signal_connect (GTK_WIDGET (self), "map",
      G_CALLBACK (hack_remap_label_widget), NULL);
}

static void
empathy_roster_group_dispose (GObject *object)
{
  EmpathyRosterGroup *self = EMPATHY_ROSTER_GROUP (object);
  void (*chain_up) (GObject *) =
      ((GObjectClass *) empathy_roster_group_parent_class)->dispose;

  tp_clear_pointer (&self->priv->widgets, g_hash_table_unref);

  if (chain_up != NULL)
    chain_up (object);
}

static void
empathy_roster_group_finalize (GObject *object)
{
  EmpathyRosterGroup *self = EMPATHY_ROSTER_GROUP (object);
  void (*chain_up) (GObject *) =
      ((GObjectClass *) empathy_roster_group_parent_class)->finalize;

  g_free (self->priv->name);
  g_free (self->priv->icon_name);

  if (chain_up != NULL)
    chain_up (object);
}

static void
empathy_roster_group_class_init (
    EmpathyRosterGroupClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);
  GParamSpec *spec;

  oclass->get_property = empathy_roster_group_get_property;
  oclass->set_property = empathy_roster_group_set_property;
  oclass->constructed = empathy_roster_group_constructed;
  oclass->dispose = empathy_roster_group_dispose;
  oclass->finalize = empathy_roster_group_finalize;

  spec = g_param_spec_string ("name", "Name",
      "Group name",
      NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (oclass, PROP_NAME, spec);

  spec = g_param_spec_string ("icon", "Icon",
      "Icon name",
      NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (oclass, PROP_ICON, spec);

  g_type_class_add_private (klass, sizeof (EmpathyRosterGroupPriv));
}

static void
empathy_roster_group_init (EmpathyRosterGroup *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
      EMPATHY_TYPE_ROSTER_GROUP, EmpathyRosterGroupPriv);

  self->priv->widgets = g_hash_table_new (NULL, NULL);
}

GtkWidget *
empathy_roster_group_new (const gchar *name,
    const gchar *icon)
{
  return g_object_new (EMPATHY_TYPE_ROSTER_GROUP,
      "name", name,
      "icon", icon,
      "use-markup", TRUE,
      "expanded", TRUE,
      NULL);
}

const gchar *
empathy_roster_group_get_name (EmpathyRosterGroup *self)
{
  return self->priv->name;
}

guint
empathy_roster_group_add_widget (EmpathyRosterGroup *self,
    GtkWidget *widget)
{
  g_return_val_if_fail (GTK_IS_WIDGET (widget), 0);

  g_hash_table_add (self->priv->widgets, widget);

  return empathy_roster_group_get_widgets_count (self);
}

guint
empathy_roster_group_remove_widget (EmpathyRosterGroup *self,
    GtkWidget *widget)
{
  g_return_val_if_fail (GTK_IS_WIDGET (widget), 0);

  g_hash_table_remove (self->priv->widgets, widget);

  return empathy_roster_group_get_widgets_count (self);
}

guint
empathy_roster_group_get_widgets_count (EmpathyRosterGroup *self)
{
  return g_hash_table_size (self->priv->widgets);
}

GList *
empathy_roster_group_get_widgets (EmpathyRosterGroup *self)
{
  return g_hash_table_get_keys (self->priv->widgets);
}
