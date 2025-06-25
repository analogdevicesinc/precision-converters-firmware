/*************************************************************************//**
 *   @file   adi_version.h
 *   @brief  ADI Version macros
******************************************************************************
* Copyright (c) 2025 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef ADI_VERSION_H
#define ADI_VERSION_H

/* Quality Levels
 * Dev: Unstable, in development phase.
 * Alpha: Does not contain all features, may contain bugs, not fully tested.
 * Beta: Supports all features, may contain bugs. Suitable for testing.
 * Release candidate: Stable, with all features. May undergo final testing.
 *                    Intended for production use.
 * Release to Manufacturing: Ready for production after extensive testing.
 * General Availability: Stable, reliable version. Intended for public use.
 */
#define QUALITY_LEVEL_DEV 	"d"
#define	QUALITY_LEVEL_ALPHA	"a"
#define	QUALITY_LEVEL_BETA 	"b"
#define	QUALITY_LEVEL_RC 	"rc"
#define	QUALITY_LEVEL_RTM 	"rtm"
#define	QUALITY_LEVEL_GA 	"ga"

/* Construct firmware_version string
 * Format: v<Major>.<Minor>.<Patch>-<Quality Level>.<State>
 *
 * Major: Significant/Breaking changes.
 * Minor: New features without breaking existing functionality.
 * Patch: Bug fixes, minor improvements.
 * Quality Level: Current quality level.
 * State: Indicate the number of internally released versions at given quality level.
 *        e.g., "0" for initial release, "1" for next iteration, etc.
 */
#define ADI_CONSTRUCT_VERSION(major, minor, patch, quality, state) \
		("v" STR(major) "." STR(minor) "." STR(patch) "-" quality "." STR(state))

#endif /* ADI_VERSION_H */
