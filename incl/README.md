# <img src="../logo.png" width="30" alt="The mod's logo." /> Avalanche Index
Official Avalanche levels and profiles.

### Development
###### Work with the mod directly.
You can access data from the Avalanche Index by including the [`Avalanche.hpp`](Avalanche.hpp) file in your code. Make sure to use the `avalanche` namespace to directly access all the needed classes and values.

```cpp
#include <cubicstudios.avalancheindex/Avalanche.hpp>

using namespace avalanche;
```

Use the `Handler` class's functions through `Handler::get()` to access functions that fetch saved data on badges and levels. Some fields may contain data based in `Project::Type` and `Profile::Badge` enum classes.

```cpp
using namespace avalanche;

class $modify(ProfilePage)
{
	void loadPageFromUserInfo(GJUserScore *user)
	{
		ProfilePage::loadPageFromUserInfo(user);

		Handler getHandler = Handler::get();
		Profile avalUser = getHandler.GetProfile(user->m_accountID);

		if (avalUser.badge != Profile::Badge::NONE)
        {
            log::info("{} is an Avalanche member! Hooray!", avalUser.name);
        };
	};
};
```

You are not required to update data manually as this mod does it by itself! Happy modding!