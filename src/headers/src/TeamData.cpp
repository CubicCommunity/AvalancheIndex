#include "../TeamData.hpp"

#include "../../../incl/Avalanche.hpp"

#include <string>
#include <map>

#include <Geode/Geode.hpp>

#include <Geode/utils/web.hpp>

using namespace geode::prelude;
using namespace avalanche;

using namespace TeamData;

void TeamData::getBadgeInfo(std::string badge_ID)
{
    if (badge_ID == Badges::badgeStringID[Profile::Badge::CUBIC])
    {
        geode::createQuickPopup(
            "Cubic Studios",
            "This user is a <cy>staff member</c> of <cj>Cubic Studios</c>. They partake in the activities of a department of Cubic, and may supervise or join projects such as <cl>Avalanche</c>.",
            "OK", "Learn More",
            [](auto, bool btn2)
            {
                if (btn2)
                {
                    web::openLinkInBrowser("https://www.cubicstudios.xyz/");
                };
            });
    }
    else if (badge_ID == Badges::badgeStringID[Profile::Badge::DIRECTOR])
    {
        geode::createQuickPopup(
            "Avalanche Director",
            "This user is the <co>director</c> of <cl>Avalanche</c>. They run the whole team.",
            "OK", "Learn More",
            [](auto, bool btn2)
            {
                if (btn2)
                {
                    web::openLinkInBrowser("https://avalanche.cubicstudios.xyz/");
                };
            });
    }
    else if (badge_ID == Badges::badgeStringID[Profile::Badge::MANAGER])
    {
        geode::createQuickPopup(
            "Avalanche Manager",
            "This user is a <cy>manager</c> of <cl>Avalanche</c>. They manage team projects and collaborations.",
            "OK", "Learn More",
            [](auto, bool btn2)
            {
                if (btn2)
                {
                    web::openLinkInBrowser("https://avalanche.cubicstudios.xyz/");
                };
            });
    }
    else if (badge_ID == Badges::badgeStringID[Profile::Badge::MEMBER])
    {
        geode::createQuickPopup(
            "Avalanche Team Member",
            "This user is a <cg>member</c> of <cl>Avalanche</c>. They partake in team projects and collaborations.",
            "OK", "Learn More",
            [](auto, bool btn2)
            {
                if (btn2)
                {
                    web::openLinkInBrowser("https://avalanche.cubicstudios.xyz/");
                };
            });
    }
    else if (badge_ID == Badges::badgeStringID[Profile::Badge::COLLABORATOR])
    {
        geode::createQuickPopup(
            "Team Collaborator",
            "This user is a <cg>collaborator</c> of <cl>Avalanche</c>. They've directly worked on the crew's or team's projects as an outsider.",
            "OK", "Learn More",
            [](auto, bool btn2)
            {
                if (btn2)
                {
                    web::openLinkInBrowser("https://avalanche.cubicstudios.xyz/");
                };
            });
    }
    else
    {
        geode::createQuickPopup(
            "Oops!",
            "This badge has <cr>no available information</c>. This is likely unintentional, please report it as an issue in the mod's repository.",
            "OK", "Report",
            [](auto, bool btn2)
            {
                if (btn2)
                {
                    web::openLinkInBrowser("https://www.github.com/CubicCommunity/AvalancheIndex/issues/");
                };
            });
    };
};

std::map<Profile::Badge, std::string> Badges::badgeStringID{
    {Profile::Badge::CUBIC, "cubic-studios"},
    {Profile::Badge::DIRECTOR, "director"},
    {Profile::Badge::MANAGER, "team-manager"},
    {Profile::Badge::MEMBER, "team-member"},
    {Profile::Badge::COLLABORATOR, "collaborator"},
};

std::map<std::string, std::string> Badges::badgeSpriteName{
    {Badges::badgeStringID[Profile::Badge::CUBIC], "cubic-studios.png"_spr},
    {Badges::badgeStringID[Profile::Badge::DIRECTOR], "director.png"_spr},
    {Badges::badgeStringID[Profile::Badge::MANAGER], "team-manager.png"_spr},
    {Badges::badgeStringID[Profile::Badge::MEMBER], "team-member.png"_spr},
    {Badges::badgeStringID[Profile::Badge::COLLABORATOR], "collaborator.png"_spr},
};

std::map<std::string, Color> Badges::badgeColor{
    {Badges::badgeStringID[Profile::Badge::CUBIC], {10, 247, 247}},
    {Badges::badgeStringID[Profile::Badge::DIRECTOR], {150, 175, 255}}, // modified to be brighter than official branding
    {Badges::badgeStringID[Profile::Badge::MANAGER], {127, 148, 255}},  // modified to be brighter than official branding
    {Badges::badgeStringID[Profile::Badge::MEMBER], {191, 201, 255}},   // modified to be brighter than official branding
    {Badges::badgeStringID[Profile::Badge::COLLABORATOR], {200, 200, 200}},
};

// badge button event
void Badges::onInfoBadge(CCObject *sender)
{
    // gets the node that triggered the function
    auto nodeObject = as<CCNode *>(sender);
    auto badge_ID = nodeObject->getID();

    TeamData::getBadgeInfo(badge_ID);
};