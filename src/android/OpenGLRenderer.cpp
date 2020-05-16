#include "../GPU3D.h"
#include "../types.h"

namespace GPU3D {
    namespace GLRenderer {
        bool Init() {
            return true;
        }
        void DeInit() {}
        void Reset() {}

        void UpdateDisplaySettings() {}

        void RenderFrame() {}
        void PrepareCaptureFrame() {}
        u32* GetLine(int line) {
            return NULL;
        }
        void SetupAccelFrame() {}
    }
}
