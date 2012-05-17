#include "config.h"

#include "empathy-roster-item.h"

#include <telepathy-glib/util.h>

#include <libempathy-gtk/empathy-images.h>
#include <libempathy-gtk/empathy-ui-utils.h>

G_DEFINE_TYPE (EmpathyRosterItem, empathy_roster_item, GTK_TYPE_BOX)

#define AVATAR_SIZE 48

enum
{
  PROP_INDIVIDIUAL = 1,
  N_PROPS
};

/*
enum
{
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];
*/

struct _EmpathyRosterItemPriv
{
  FolksIndividual *individual;

  GtkWidget *avatar;
  GtkWidget *alias;
  GtkWidget *presence_msg;
  GtkWidget *presence_icon;
};

static void
empathy_roster_item_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  EmpathyRosterItem *self = EMPATHY_ROSTER_ITEM (object);

  switch (property_id)
    {
      case PROP_INDIVIDIUAL:
        g_value_set_object (value, self->priv->individual);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
empathy_roster_item_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  EmpathyRosterItem *self = EMPATHY_ROSTER_ITEM (object);

  switch (property_id)
    {
      case PROP_INDIVIDIUAL:
        g_assert (self->priv->individual == NULL); /* construct only */
        self->priv->individual = g_value_dup_object (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
avatar_loaded_cb (GObject *source,
    GAsyncResult *result,
    gpointer user_data)
{
  TpWeakRef *wr = user_data;
  EmpathyRosterItem *self;
  GdkPixbuf *pixbuf;

  self = tp_weak_ref_dup_object (wr);
  if (self == NULL)
    goto out;

  pixbuf = empathy_pixbuf_avatar_from_individual_scaled_finish (
      FOLKS_INDIVIDUAL (source), result, NULL);

  if (pixbuf == NULL)
    {
      pixbuf = empathy_pixbuf_from_icon_name_sized (
          EMPATHY_IMAGE_AVATAR_DEFAULT, AVATAR_SIZE);
    }

  gtk_image_set_from_pixbuf (GTK_IMAGE (self->priv->avatar), pixbuf);
  g_object_unref (pixbuf);

  g_object_unref (self);

out:
  tp_weak_ref_destroy (wr);
}

static void
update_avatar (EmpathyRosterItem *self)
{
  empathy_pixbuf_avatar_from_individual_scaled_async (self->priv->individual,
      AVATAR_SIZE, AVATAR_SIZE, NULL, avatar_loaded_cb,
      tp_weak_ref_new (self, NULL, NULL));
}

static void
avatar_changed_cb (FolksIndividual *individual,
    GParamSpec *spec,
    EmpathyRosterItem *self)
{
  update_avatar (self);
}

static void
update_alias (EmpathyRosterItem *self)
{
  const gchar *alias;

  alias = folks_alias_details_get_alias (FOLKS_ALIAS_DETAILS (
        self->priv->individual));

  gtk_label_set_text (GTK_LABEL (self->priv->alias), alias);
}

static void
alias_changed_cb (FolksIndividual *individual,
    GParamSpec *spec,
    EmpathyRosterItem *self)
{
  update_alias (self);
}

static void
update_presence_msg (EmpathyRosterItem *self)
{
  const gchar *msg;

  msg = folks_presence_details_get_presence_message (
      FOLKS_PRESENCE_DETAILS (self->priv->individual));

  gtk_label_set_text (GTK_LABEL (self->priv->presence_msg), msg);
}

static void
presence_message_changed_cb (FolksIndividual *individual,
    GParamSpec *spec,
    EmpathyRosterItem *self)
{
  update_presence_msg (self);
}

static void
update_presence_icon (EmpathyRosterItem *self)
{
  const gchar *icon;

  icon = empathy_icon_name_for_individual (self->priv->individual);

  gtk_image_set_from_icon_name (GTK_IMAGE (self->priv->presence_icon), icon,
      GTK_ICON_SIZE_MENU);
}

static void
presence_status_changed_cb (FolksIndividual *individual,
    GParamSpec *spec,
    EmpathyRosterItem *self)
{
  update_presence_icon (self);
}

static void
empathy_roster_item_constructed (GObject *object)
{
  EmpathyRosterItem *self = EMPATHY_ROSTER_ITEM (object);
  void (*chain_up) (GObject *) =
      ((GObjectClass *) empathy_roster_item_parent_class)->constructed;

  if (chain_up != NULL)
    chain_up (object);

  g_assert (FOLKS_IS_INDIVIDUAL (self->priv->individual));

  tp_g_signal_connect_object (self->priv->individual, "notify::avatar",
      G_CALLBACK (avatar_changed_cb), self, 0);
  tp_g_signal_connect_object (self->priv->individual, "notify::alias",
      G_CALLBACK (alias_changed_cb), self, 0);
  tp_g_signal_connect_object (self->priv->individual,
      "notify::presence-message",
      G_CALLBACK (presence_message_changed_cb), self, 0);
  tp_g_signal_connect_object (self->priv->individual, "notify::presence-status",
      G_CALLBACK (presence_status_changed_cb), self, 0);

  update_avatar (self);
  update_alias (self);
  update_presence_msg (self);
  update_presence_icon (self);
}

static void
empathy_roster_item_dispose (GObject *object)
{
  EmpathyRosterItem *self = EMPATHY_ROSTER_ITEM (object);
  void (*chain_up) (GObject *) =
      ((GObjectClass *) empathy_roster_item_parent_class)->dispose;

  g_clear_object (&self->priv->individual);

  if (chain_up != NULL)
    chain_up (object);
}

static void
empathy_roster_item_finalize (GObject *object)
{
  //EmpathyRosterItem *self = EMPATHY_ROSTER_ITEM (object);
  void (*chain_up) (GObject *) =
      ((GObjectClass *) empathy_roster_item_parent_class)->finalize;

  if (chain_up != NULL)
    chain_up (object);
}

static void
empathy_roster_item_class_init (
    EmpathyRosterItemClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);
  GParamSpec *spec;

  oclass->get_property = empathy_roster_item_get_property;
  oclass->set_property = empathy_roster_item_set_property;
  oclass->constructed = empathy_roster_item_constructed;
  oclass->dispose = empathy_roster_item_dispose;
  oclass->finalize = empathy_roster_item_finalize;

  spec = g_param_spec_object ("individual", "Individual",
      "FolksIndividual",
      FOLKS_TYPE_INDIVIDUAL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (oclass, PROP_INDIVIDIUAL, spec);

  g_type_class_add_private (klass, sizeof (EmpathyRosterItemPriv));
}

static void
empathy_roster_item_init (EmpathyRosterItem *self)
{
  GtkWidget *box;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
      EMPATHY_TYPE_ROSTER_ITEM, EmpathyRosterItemPriv);

  gtk_widget_set_size_request (GTK_WIDGET (self), 300, 64);

  /* Avatar */
  self->priv->avatar = gtk_image_new ();

  gtk_box_pack_start (GTK_BOX (self), self->priv->avatar, FALSE, FALSE, 0);
  gtk_widget_show (self->priv->avatar);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  /* Alias */
  self->priv->alias = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (self->priv->alias), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (box), self->priv->alias, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (self), box, TRUE, TRUE, 0);

  /* Presence */
  self->priv->presence_msg = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (self->priv->presence_msg), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (box), self->priv->presence_msg, TRUE, TRUE, 0);

  gtk_widget_show_all (box);

  /* Presence icon */
  self->priv->presence_icon = gtk_image_new ();

  gtk_box_pack_start (GTK_BOX (self), self->priv->presence_icon,
      FALSE, FALSE, 0);
  gtk_widget_show (self->priv->presence_icon);
}

GtkWidget *
empathy_roster_item_new (FolksIndividual *individual)
{
  g_return_val_if_fail (FOLKS_IS_INDIVIDUAL (individual), NULL);

  return g_object_new (EMPATHY_TYPE_ROSTER_ITEM,
      "individual", individual,
      "spacing", 8,
      NULL);
}

FolksIndividual *
empathy_roster_item_get_individual (EmpathyRosterItem *self)
{
  return self->priv->individual;
}
