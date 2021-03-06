#pragma once

namespace World {
    const int up_idx = 1;
    const int front_idx = 2;
    const glm::vec4 up(0, 1, 0, 0);
    const glm::vec4 front(0, 0, 1, 0);

    const float gravityMag = 9.8f;
    const glm::vec4 gravity = up * -gravityMag;
}
