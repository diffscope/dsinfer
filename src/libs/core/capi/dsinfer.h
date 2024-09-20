#ifndef DSINFER_H
#define DSINFER_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DSLoader DSLoader;

typedef struct DSVersion {
    int major;
    int minor;
    int patch;
    int tweak;
} DSVersion;

typedef struct DSDependency {
    const char *id;
    DSVersion version;
} DSDependency;

enum DSContributeType {
    DSC_Inference,
    DSC_Singer,
};

typedef struct DSContribute {
    int type;
    const char *id;
    const char *path;
} DSContribute;

typedef struct DSLibrary {
    const char *id;
    DSVersion version;
    DSVersion compatVersion;
    const char *vendor;
    const char *copyright;
    const char *description;
    const char *url;
    DSDependency **dependencies;
    DSContribute **contributes[2];
} DSLibrary;

typedef struct DSInferenceContribute {
    DSContribute base;
    const char *name;
    int level;
    const char *schema;        // json
    const char *configuration; // json
} DSInferenceContribute;

typedef struct DSSingerPresetItem {
    const char *library;
    const char *inference;
    const char *options; // json
} DSSingerPresetItem;

typedef struct DSSingerContribute {
    DSContribute base;
    const char *name;
    const char *avatar;
    const char *background;
    const char *demoAudio;
    DSSingerPresetItem **preset;
} DSSinger;

typedef struct DSImage {
    const char *path;
    bool running;
} DSImage;

typedef struct DSInterpreter {
    const char *(*key)();
    bool (*load)(DSInferenceContribute *contribute, const char **error);
    void (*start)(const char *input);
} DSInterpreter;

DSLoader *DS_CreateLoadeer(void);
void DS_FreeLoader(DSLoader *loader);

void DS_AddInterpreterPath(DSLoader *loader, const char *path);
void DS_SetInterpreterPaths(DSLoader *loader, const char **paths);

DSLibrary *DS_LoadLibrary(DSLoader *loader, const char *path, char **error = NULL);
void DS_FreeLibrary(DSLoader *loader, DSLibrary *handle);

DSLibrary *DS_GetLibrary(DSLoader *loader, const char *id, const DSVersion *version);
int DS_GetLibraries(DSLoader *loader, const char *id, DSLibrary **buf);



#ifdef __cplusplus
}
#endif

#endif // DSINFER_H
