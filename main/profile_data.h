// profile_data.h - GENERATED FILE, DO NOT EDIT BY HAND.
// Source: data/profile.json  |  Regenerate: python tools/gen_profile.py
// Edit data/profile.json instead, then rebuild the firmware.
// Entry text is pre-wrapped to fit a 160px column.
#pragma once

// Geometry shared with main/passport_ui.c and tools/check_layout.py.
#define AZMI_CONTENT_W     184
#define AZMI_ENTRY_LINES   3
#define AZMI_ITEMS_PER_PAGE 2
#define AZMI_LINE_H        19
#define AZMI_QR_SIDE       135
#define AZMI_QR_SCALE     3
#define AZMI_QR_QUIET     4
#define AZMI_EMAIL_LINE_W 184
#define AZMI_CARD_PAD_Y   8
#define AZMI_CARD_GAP     8
#define AZMI_PERIOD_H     17

#define AZMI_NAME        "Dr. Azmi Bin Mohd Yusof"
#define AZMI_NAME_DISPLAY "Dr. Azmi Bin Mohd\nYusof"
#define AZMI_TITLE       "Senior Lecturer"
#define AZMI_INSTITUTION "Xiamen University Malaysia"
#define AZMI_INST_LINE1  "Xiamen University"
#define AZMI_INST_LINE2  "Malaysia"
#define AZMI_EMAIL       "azmi.mohdyusof@xmu.edu.my"
#define AZMI_OFFICE      "A1#348"
#define AZMI_PROFILE_URL "https://www.xmu.edu.my/staff/dr-azmi-bin-mohd-yusof"

#define AZMI_EMAIL_L1   "azmi.mohdyusof"
#define AZMI_EMAIL_L2   "@xmu.edu.my"

#define AZMI_TAG_COUNT 4
static const char *const AZMI_TAGS[AZMI_TAG_COUNT] = {
    "XR",
    "VR",
    "MR",
    "AR",
};

#define AZMI_JOURNEY_PAGE_COUNT 4
static const char *const AZMI_JOURNEY_PAGE_TITLES[AZMI_JOURNEY_PAGE_COUNT] = {
    "EDUCATION",
    "APPOINTMENTS",
    "EARLY CAREER",
    "FOUNDATION",
};
static const char *const AZMI_JOURNEY_PERIODS[AZMI_JOURNEY_PAGE_COUNT][2] = {
    { "2013", "1988-86" },
    { "2025", "2000-25" },
    { "1991-2013", "1990-2000" },
    { "1988-89", "1986-87" },
};
static const char *const AZMI_JOURNEY_LINES[AZMI_JOURNEY_PAGE_COUNT][2][3] = {
    { { "PhD, HCI & 3D", "Modelling", "Univ. of Waikato, NZ" }, { "MSc & BSc, Comp. Sci.", "W. Michigan, USA", "" } },
    { { "Senior Lecturer", "Xiamen Univ.,", "Malaysia" }, { "Principal Lecturer", "UTN, Malaysia", "" } },
    { { "System Developer", "Freelance, MY", "" }, { "Asst. Manager, IT", "Dept.", "Sony TV Industries" } },
    { { "Programmer", "Datex Micro", "Computer, USA" }, { "System Developer", "Univ. Hospital, MI", "" } },
};

#define AZMI_RESEARCH_PAGE_COUNT 4
static const char *const AZMI_RESEARCH_PAGE_TITLES[AZMI_RESEARCH_PAGE_COUNT] = {
    "FOCUS AREAS",
    "ENERGY TRAINING",
    "STRESS THERAPY",
    "VR FOR LEARNING",
};
static const char *const AZMI_RESEARCH_PERIODS[AZMI_RESEARCH_PAGE_COUNT][2] = {
    { "XR", "VR" },
    { "2018-19", "2018-19" },
    { "2014-16", "2019" },
    { "2020-21", "2022-23" },
};
static const char *const AZMI_RESEARCH_LINES[AZMI_RESEARCH_PAGE_COUNT][2][3] = {
    { { "Extended Reality", "Education &", "Healthcare" }, { "Stress Therapy", "Immersive", "intervention" } },
    { { "MR for Power Station", "Operator training", "" }, { "VR Training of Gas", "Turbine for TNB", "" } },
    { { "Stress Therapy in VR", "PhD research, ITEX", "Gold" }, { "VR Therapy", "Innovation", "TNB Fund, 1st Prize" } },
    { { "VR for TVET", "Maintenance training", "" }, { "Cycling in VR", "Motion platform,", "Silver" } },
};

#define AZMI_AWARDS_PAGE_COUNT 3
static const char *const AZMI_AWARDS_PAGE_TITLES[AZMI_AWARDS_PAGE_COUNT] = {
    "GOLD",
    "GOLD 2",
    "SILVER",
};
static const char *const AZMI_AWARDS_PERIODS[AZMI_AWARDS_PAGE_COUNT][2] = {
    { "2019", "2018" },
    { "2014", "2015" },
    { "2023", "" },
};
static const char *const AZMI_AWARDS_LINES[AZMI_AWARDS_PAGE_COUNT][2][3] = {
    { { "ITEX Gold", "VRTurbine Education", "4.0" }, { "PERINTIS Gold", "VRTurbine", "" } },
    { { "ITEX Gold", "Stress Treatment, VR", "" }, { "TNB First Prize", "Open Innovation", "" } },
    { { "PENCIPTA Silver", "Immersive Bicycle", "" }, { "", "", "" } },
};

#define AZMI_IMPACT_PAGE_COUNT 3
static const char *const AZMI_IMPACT_PAGE_TITLES[AZMI_IMPACT_PAGE_COUNT] = {
    "RESEARCH IMPACT",
    "EDUCATION",
    "SCOPE",
};
static const char *const AZMI_IMPACT_PERIODS[AZMI_IMPACT_PAGE_COUNT][2] = {
    { "STRESS", "TRAINING" },
    { "TVET", "TERTIARY" },
    { "XR", "HEALTH" },
};
static const char *const AZMI_IMPACT_LINES[AZMI_IMPACT_PAGE_COUNT][2][3] = {
    { { "VR therapy for", "anxiety", "Clinical pilot, MY" }, { "Operator training", "Power utility sector", "" } },
    { { "Maintenance skills", "VR-based curriculum", "" }, { "HCI & 3D modelling", "Postgrad supervision", "" } },
    { { "Education 4.0", "Immersive", "curriculum" }, { "Clinical therapy", "Mental wellbeing", "" } },
};

#define AZMI_TEACHING_PAGE_COUNT 3
static const char *const AZMI_TEACHING_PAGE_TITLES[AZMI_TEACHING_PAGE_COUNT] = {
    "COURSES",
    "TEACHING",
    "SUPERVISION",
};
static const char *const AZMI_TEACHING_PERIODS[AZMI_TEACHING_PAGE_COUNT][2] = {
    { "HCI", "3D" },
    { "VR", "TECH" },
    { "PG", "UG" },
};
static const char *const AZMI_TEACHING_LINES[AZMI_TEACHING_PAGE_COUNT][2][3] = {
    { { "Human-Computer", "Interaction", "Postgraduate" }, { "3D Modelling &", "Animation", "Undergraduate" } },
    { { "Virtual Reality Lab", "Hands-on studio", "" }, { "Emerging", "Technology", "Industry-facing" } },
    { { "Postgraduate", "research", "XR & HCI topics" }, { "Final-year projects", "VR applications", "" } },
};

#define AZMI_LIST_SUBPAGES 17
