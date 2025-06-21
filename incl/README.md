# <img src="../logo.png" width="30" alt="The mod's logo." /> Avalanche Index
Official Avalanche levels and profiles.

### Development
###### Work with the mod directly.
You can access data from the Avalanche Index by including the [`Avalanche.hpp`](./Avalanche.hpp) file in your code. Make sure to use the `avalanche` namespace to directly access all the needed classes and values.

```cpp
#include <cubicstudios.avalancheindex/incl/Avalanche.hpp>

using namespace avalanche;
```

Use the `Handler` class's methods to access methods that fetch saved data on badges and levels. Some fields may contain data based in `Project::Type` and `Profile::Badge` enum classes.

> [!IMPORTANT]
> To utilize `Handler`'s methods, you **must** initialize the class with **`Handler::get()`** beforehand. Otherwise, crucial non-static methods will not be accessible.
>
> ```cpp
> Handler * getHandler = Handler::get();
> ```
>
> This will return a *pointer* - be sure to use the **arrow operator** (`->`) to access methods.


Here's some sample code to fetch and handle data from an Avalanche profile.
```cpp
using namespace avalanche;

Handler * getHandler = Handler::get();

class $modify(ProfilePage) {
    void loadPageFromUserInfo(GJUserScore * user) {
        ProfilePage::loadPageFromUserInfo(user);

        Profile avalUser = getHandler->GetProfile(user->m_accountID);

        if (avalUser.badge != Profile::Badge::NONE) {
            log::info("{} is an Avalanche member! Hooray!", avalUser.name);

            if (avalUser.badge == Profile::Badge::DIRECTOR) {
                log::info("Woah! {} is the director of Avalanche!", avalUser.name);
            };
        };
    };
};
```

Here's some sample code to fetch and handle data from an Avalanche project.
```cpp
using namespace avalanche;

Handler * getHandler = Handler::get();

class $modify(Level, LevelCell) {
	void loadFromLevel(GJGameLevel * level) {
		LevelCell::loadFromLevel(level);

		Project avalLevel = getHandler->GetProject(level->m_levelID.value());

		if (avalLevel.type == Project::Type::TEAM) {
			log::info("The level '{}' is an Avalanche project hosted by {}!", avalLevel.name, avalLevel.host);
		} else if (avalLevel.type == Project::Type::EVENT) {
			log::info("The level '{}' by {} won an Avalanche event! Yay!", avalLevel.name, avalLevel.host);
		};
	};
};
```

You are not required to update data manually as this mod does it by itself! Happy modding!

### Users
###### Regular users of this mod.
**[📱 Learn about this mod](../)**