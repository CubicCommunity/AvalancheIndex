#include <Geode/Geode.hpp>
#include <Geode/DefaultInclude.hpp>

#include <Geode/binding/PlatformToolbox.hpp>

#include <geode.custom-keybinds/include/Keybinds.hpp>

using namespace geode::prelude;
using namespace keybinds;

$execute
{
    BindManager *bind = BindManager::get();

    bind->registerBindable({"open-featured"_spr,
                            "Featured Project",
                            "Open the overlay showing the featured Avalanche project.",
                            {Keybind::create(KEY_Divide, Modifier::None)},
                            "Avalanche/Index"});

    bind->registerBindable({"badge-info"_spr,
                            "View Badge Information",
                            "Display a pop-up that shows more information about the Avalanche badge while viewing a user's profile.",
                            {Keybind::create(KEY_Multiply, Modifier::None)},
                            "Avalanche/Index"});
};