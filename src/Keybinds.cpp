#include <Geode/Geode.hpp>
#include <Geode/DefaultInclude.hpp>

#include <geode.custom-keybinds/include/Keybinds.hpp>

using namespace geode::prelude;
using namespace keybinds;

$execute
{
    auto bind = BindManager::get();

    bind->registerBindable({"k_open-featured"_spr,
                            "Featured Project",
                            "Open the overlay showing the featured Avalanche project.",
                            {Keybind::create(enumKeyCodes::KEY_Divide, Modifier::None)},
                            "Avalanche/Index"});

    bind->registerBindable({"k_badge-info"_spr,
                            "View Badge Information",
                            "Display a pop-up that shows more information about the Avalanche badge while viewing a user's profile.",
                            {Keybind::create(enumKeyCodes::KEY_Multiply, Modifier::None)},
                            "Avalanche/Index"});
};