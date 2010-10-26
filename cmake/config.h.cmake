#ifndef CONFIG_H
#define CONFIG_H

/* With crash button (tools->crash) */
#cmakedefine WITH_CRASHBUTTON

/* With crash handler */
#cmakedefine WITH_CRASHHANDLER

/* With crash handler GUI */
#cmakedefine WITH_CRASHHANDLER_GUI

/* Absolute path to plugins */
#cmakedefine VLMC_EFFECTS_DIR "@VLMC_EFFECTS_DIR@"

/* Host computer name */
#cmakedefine HOSTNAME "@HOSTNAME@"

/* OS Name */
#cmakedefine SYSNAME "@SYSNAME@"

/* VLMC's major version */
#cmakedefine PROJECT_VERSION_MAJOR "@PROJECT_VERSION_MAJOR@"

/* VLMC's version */
#cmakedefine PROJECT_VERSION "@PROJECT_VERSION@"

/* Version Codename */
#cmakedefine CODENAME "@CODENAME@"

/* VLMC Copyright string */
#cmakedefine PROJECT_COPYRIGHT "@PROJECT_COPYRIGHT@"

/* VLMC Contact email */
#cmakedefine PROJECT_CONTACT "@PROJECT_CONTACT@"

/* VideoLAN website */
#cmakedefine ORG_WEBSITE "@ORG_WEBSITE@"

/* GUI application ? */
#cmakedefine WITH_GUI

#endif //CONFIG_H
