#pragma once

#ifdef DEDSEC_OS
#include <Arduino.h>

void dedsecBootStage(const char *stage, unsigned step);
void dedsecDashboard();
void dedsecWifiSurvey();
void dedsecSaveDiagnostic();
bool dedsecWriteDiagnostic(Print &output);
void dedsecSettings();
void dedsecHardwareInfo();
const char *dedsecHardwareRequirement(const String &name);
bool dedsecConfirmMenuOperation(const String &label, bool hardwareEntry = false);
#endif
