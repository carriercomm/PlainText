#include "setpropcommand.h"

#include "engine/item.h"
#include "engine/util.h"


SetPropCommand::SetPropCommand(Player *character, QObject *parent) :
    AdminCommand(character, parent) {

    setDescription("Set the value of some object's property.\n"
                   "\n"
                   "Usage: set-prop <object-name> [#] <property-name> <value>");
}

SetPropCommand::~SetPropCommand() {
}

void SetPropCommand::execute(const QString &command) {

    setCommand(command);

    /*QString alias = */takeWord();

    GameObjectPtrList objects = takeObjects(currentArea()->objects());
    if (!requireUnique(objects, "Object not found.", "Object is not unique.")) {
        return;
    }

    QString propertyName = Util::toCamelCase(takeWord());
    QString value = takeRest();

    QVariant variant = objects[0]->property(propertyName.toAscii().constData());
    switch (variant.type()) {
        case QVariant::Bool:
            variant = (value == "true");
            break;
        case QVariant::Int:
            variant = value.toInt();
            break;
        case QVariant::String:
            variant = value.replace("\\n", "\n");
            break;
        case QVariant::UserType:
            if (variant.userType() == QMetaType::type("GameObjectPtr")) {
                variant = QVariant::fromValue(GameObjectPtr::fromString(value));
                break;
            } else if (variant.userType() == QMetaType::type("GameObjectPtrList")) {
                try {
                    GameObjectPtrList pointerList;
                    foreach (QString string, value.split(' ')) {
                        pointerList << GameObjectPtr::fromString(string);
                    }
                    variant = QVariant::fromValue(pointerList);
                } catch (const GameException &exception) {
                    player()->send(exception.what());
                }
                break;
            }
        default:
            player()->send(QString("Setting property %1 is not supported.").arg(propertyName));
            return;
    }

    if (variant.isValid()) {
        objects[0]->setProperty(propertyName.toAscii().constData(), variant);

        player()->send(QString("Property %1 modified.").arg(propertyName));

        Item *item = objects[0].cast<Item *>();
        if (item && (propertyName == "name" || propertyName == "plural" || propertyName == "indefiniteArticle")) {
            player()->send(QString("New forms: one %1, two %2, %3 %4.").arg(item->name(),
                                                                               item->plural(),
                                                                               item->indefiniteArticle(),
                                                                               item->name()));
        }
    }
}