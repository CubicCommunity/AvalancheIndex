#include <Geode/Geode.hpp>
#include <Geode/DefaultInclude.hpp>

#include <Geode/binding/PlatformToolbox.hpp>

#include <geode.custom-keybinds/include/Keybinds.hpp>

using namespace geode::prelude;
using namespace keybinds;

$execute
{
    BindManager *bind = BindManager::get();

    auto key_openfeatured = KEY_Divide;
    auto key_badgeinfo = KEY_Multiply;

    if (PlatformToolbox::isControllerConnected())
    {
        key_openfeatured = CONTROLLER_Up;
        key_badgeinfo = CONTROLLER_X;
    };

    bind->registerBindable({"open-featured"_spr,
                            "Featured Project",
                            "Open the overlay showing the featured Avalanche project.",
                            {Keybind::create(key_openfeatured, Modifier::None)},
                            "Avalanche/Index"});

    bind->registerBindable({"badge-info"_spr,
                            "View Badge Information",
                            "Display a pop-up that shows more information about the Avalanche badge while viewing a user's profile.",
                            {Keybind::create(key_badgeinfo, Modifier::None)},
                            "Avalanche/Index"});
};