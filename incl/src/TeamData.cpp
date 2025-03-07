#include "../TeamData.hpp"

#include <string>
#include <map>

#include <Geode/Geode.hpp>

#include <Geode/utils/web.hpp>

using namespace geode::prelude;
using namespace TeamData;

void TeamData::getBadgeInfo(std::string badge_ID)
{
    if (badge_ID == Badges::badgeStringID[BadgeID::CUBIC])
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
    else if (badge_ID == Badges::badgeStringID[BadgeID::DIRECTOR])
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
    else if (badge_ID == Badges::badgeStringID[BadgeID::MANAGER])
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
    else if (badge_ID == Badges::badgeStringID[BadgeID::MEMBER])
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
    else if (badge_ID == Badges::badgeStringID[BadgeID::COLLABORATOR])
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

std::map<BadgeID, std::string> Badges::badgeStringID{
    {BadgeID::CUBIC, "cubic-studios"},
    {BadgeID::DIRECTOR, "director"},
    {BadgeID::MANAGER, "team-manager"},
    {BadgeID::MEMBER, "team-member"},
    {BadgeID::COLLABORATOR, "collaborator"},
};

std::map<std::string, std::string> Badges::badgeSpriteName{
    {Badges::badgeStringID[BadgeID::CUBIC], "cubic-studios.png"_spr},
    {Badges::badgeStringID[BadgeID::DIRECTOR], "director.png"_spr},
    {Badges::badgeStringID[BadgeID::MANAGER], "team-manager.png"_spr},
    {Badges::badgeStringID[BadgeID::MEMBER], "team-member.png"_spr},
    {Badges::badgeStringID[BadgeID::COLLABORATOR], "collaborator.png"_spr},
};

std::map<std::string, Color> Badges::badgeColor{
    {Badges::badgeStringID[BadgeID::CUBIC], {10, 247, 247}},
    {Badges::badgeStringID[BadgeID::DIRECTOR], {150, 175, 255}}, // modified to be brighter than official branding
    {Badges::badgeStringID[BadgeID::MANAGER], {127, 148, 255}},  // modified to be brighter than official branding
    {Badges::badgeStringID[BadgeID::MEMBER], {191, 201, 255}},   // modified to be brighter than official branding
    {Badges::badgeStringID[BadgeID::COLLABORATOR], {200, 200, 200}},
};

// badge button event
void Badges::onInfoBadge(CCObject *sender)
{
    // gets the node that triggered the function
    auto nodeObject = as<CCNode *>(sender);
    auto badge_ID = nodeObject->getID();

    TeamData::getBadgeInfo(badge_ID);
};