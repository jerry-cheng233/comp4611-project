#pragma once

#define DISTRIBUTION_MODE 1

#define TODO_IMPLEMENT_ME \
        ((!DISTRIBUTION_MODE) || \
            (std::cout << "❌ There's still a TODO on line " << __LINE__ << " of " << __FILE__ << ". If you've implemented the functionality, remove the TODO." << std::endl, std::exit(1), false))
