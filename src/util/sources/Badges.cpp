#include "../Badges.hpp"

#include <string>
#include <map>

#include <Geode/Geode.hpp>

#include <Geode/utils/web.hpp>

using namespace geode::prelude;

std::map<Badges::BadgeID, std::string> Badges::badgeStringID{
    {Badges::BadgeID::Cubic, "cubic-studios"},
    {Badges::BadgeID::Director, "director"},
    {Badges::BadgeID::Manager, "team-manager"},
    {Badges::BadgeID::Member, "team-member"},
    {Badges::BadgeID::Collaborator, "collaborator"}};

std::map<std::string, std::string> Badges::badgeSpriteName{
    {Badges::badgeStringID[Badges::BadgeID::Cubic], "cubic-studios.png"_spr},
    {Badges::badgeStringID[Badges::BadgeID::Director], "director.png"_spr},
    {Badges::badgeStringID[Badges::BadgeID::Manager], "team-manager.png"_spr},
    {Badges::badgeStringID[Badges::BadgeID::Member], "team-member.png"_spr},
    {Badges::badgeStringID[Badges::BadgeID::Collaborator], "collaborator.png"_spr}};

// badge button event
void Badges::onInfoBadge(CCObject *sender)
{
    // gets the node that triggered the function
    auto nodeObject = as<CCNode *>(sender);
    auto badge_ID = nodeObject->getID();

    if (badge_ID == Badges::badgeStringID[Badges::BadgeID::Cubic])
    {
        geode::createQuickPopup(
            "Cubic Studios",
            "This user is a <cj>staff member</c> of <cc>Cubic Studios</c>. They partake in the activities of a department of Cubic.",
            "OK", "Learn More",
            [](auto, bool btn2)
            {
                if (btn2)
                {
                    web::openLinkInBrowser("https://www.cubicstudios.xyz/");
                };
            });
    }
    else if (badge_ID == Badges::badgeStringID[Badges::BadgeID::Director])
    {
        geode::createQuickPopup(
            "Avalanche Director",
            "This user is the <cj>director</c> of <cl>Avalanche</c>. They manage and supervise everything.",
            "OK", "Learn More",
            [](auto, bool btn2)
            {
                if (btn2)
                {
                    web::openLinkInBrowser("https://avalanche.cubicstudios.xyz/");
                };
            });
    }
    else if (badge_ID == Badges::badgeStringID[Badges::BadgeID::Manager])
    {
        geode::createQuickPopup(
            "Avalanche Manager",
            "This user is a <cy>manager</c> of <cb>Avalanche</c>. They manage group projects and collaborations.",
            "OK", "Learn More",
            [](auto, bool btn2)
            {
                if (btn2)
                {
                    web::openLinkInBrowser("https://avalanche.cubicstudios.xyz/");
                };
            });
    }
    else if (badge_ID == Badges::badgeStringID[Badges::BadgeID::Member])
    {
        geode::createQuickPopup(
            "Avalanche Team Member",
            "This user is a <cg>member</c> of <cb>Avalanche</c>. They partake in group projects and collaborations.",
            "OK", "Learn More",
            [](auto, bool btn2)
            {
                if (btn2)
                {
                    web::openLinkInBrowser("https://avalanche.cubicstudios.xyz/");
                };
            });
    }
    else if (badge_ID == Badges::badgeStringID[Badges::BadgeID::Collaborator])
    {
        geode::createQuickPopup(
            "Avalanche Collaborator",
            "This user is a <cg>collaborator</c> of <cb>Avalanche</c>. They've directly worked on the crew's or team's projects as an outsider.",
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