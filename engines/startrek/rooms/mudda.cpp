/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "startrek/startrek.h"
#include "startrek/room.h"

namespace StarTrek {

enum muddTextIds {
	TX_COMMON_IDS_OFFSET_START = 5000, // needed to distinguish common IDs
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK,
	TX_MUD0_002, TX_MUD0_018, TX_MUD0_019, TX_MUD0_020,
	TX_MUD0N002, TX_MUD0N006, TX_MUD0N011
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets muddTextOffsets[] = {
	{ TX_SPEAKER_KIRK, 2941, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 2966, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 2976, 0, 0, 0 },
	{ TX_MUD0_002, 8651, 0, 0, 0 },
	{ TX_MUD0_018, 2523, 0, 0, 0 },
	{ TX_MUD0_019, 2473, 0, 0, 0 },
	{ TX_MUD0_020, 2423, 0, 0, 0 },
	{ TX_MUD0N002, 5513, 0, 0, 0 },
	{ TX_MUD0N006, 2573, 0, 0, 0 },
	{ TX_MUD0N011, 6036, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

// The functions here used to be independently implemented in each room of the MUDD
// mission, despite being mostly the same.

void Room::muddaUseLenseOnDegrimer() {
	assert(_vm->_roomIndex >= 0 && _vm->_roomIndex <= 5);

	giveItem(OBJECT_IALIENDV);
	loseItem(OBJECT_IDEGRIME);
	loseItem(OBJECT_ILENSES);

	_awayMission->mudd.missionScore++;
	showDescription(TX_MUD0N011);
}


void Room::muddaUseAlienDevice() {
	assert(_vm->_roomIndex >= 0 && _vm->_roomIndex <= 5);

	const int deviceObjectIndices[] = { // Each room's object index for the explosion is different
		9,  // MUDD0
		13, // MUDD1
		11, // MUDD2
		11, // MUDD3
		9,  // MUDD4
		11  // MUDD5
	};

	_awayMission->disableInput = true;

	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
	loadActorStandAnim(OBJECT_KIRK);
	Common::Point pos = getActorPos(OBJECT_KIRK);
	loadActorAnimC(deviceObjectIndices[_vm->_roomIndex], "s4cbxp", pos.x, 10, &Room::muddaFiredAlienDevice);
	playVoc("EXPLO3");
}

void Room::muddaFiredAlienDevice() {
	assert(_vm->_roomIndex >= 0 && _vm->_roomIndex <= 5);

	_awayMission->disableInput = false;
	if (!_awayMission->mudd.discoveredLenseAndDegrimerFunction) {
		_awayMission->mudd.discoveredLenseAndDegrimerFunction = true;
		_awayMission->mudd.missionScore += 5; // BUGFIX: didn't happen if done in MUDD5
		showText(TX_SPEAKER_KIRK, TX_MUD0_002);
	}
}

void Room::muddaUseDegrimer() {
	assert(_vm->_roomIndex >= 0 && _vm->_roomIndex <= 5);

	showDescription(TX_MUD0N002);
}

void Room::muddaTick() {
	// This function deals with the atmosphere running out when the life support generator
	// is malfunctioning.
	// NOTE: This has been changed a bit; in the original, they would just walk to
	// a position and stay standing, though the code indicates they were supposed to
	// collapse after walking.
	// To simplify things, and since it makes more sense, now they'll just collapse on the
	// spot instead.

	assert(_vm->_roomIndex >= 0 && _vm->_roomIndex <= 5);

	const int TIMER_LENGTH = 27000;

	if (_awayMission->mudd.lifeSupportMalfunctioning) {
		if (!_awayMission->mudd.startedLifeSupportTimer) {
			_awayMission->mudd.startedLifeSupportTimer = true;
			_awayMission->mudd.lifeSupportTimer = TIMER_LENGTH;
		}
		_awayMission->mudd.lifeSupportTimer--;

		// BUGFIX: the warnings at 75%, 50%, and 25% were only voiced in MUDD0.
		if (_awayMission->mudd.lifeSupportTimer == (int)(TIMER_LENGTH * 0.25))
			showText(TX_SPEAKER_SPOCK, TX_MUD0_018);
		else if (_awayMission->mudd.lifeSupportTimer == (int)(TIMER_LENGTH * 0.5))
			showText(TX_SPEAKER_SPOCK, TX_MUD0_019);
		else if (_awayMission->mudd.lifeSupportTimer == (int)(TIMER_LENGTH * 0.75))
			showText(TX_SPEAKER_SPOCK, TX_MUD0_020);
		else if (_awayMission->mudd.lifeSupportTimer == 1) {
			_awayMission->disableInput = true;

			// In each room, the crewmen collapse in a different directions.
			// NOTE: "kgetdn" (kirk, north) doesn't work properly; files in the animation
			// are missing. It's replaced with 'e' (east) in MUDD1, MUDD3, MUDD5. Only
			// applies to Kirk, others seem fine...
			// TODO: check if this is the case across all versions...
			const char *directions[] = {
				"weseee", // KIRK
				"sewene", // SPOCK
				"nsesss", // MCCOY
				"ewesww", // REDSHIRT
			};

			for (int i = OBJECT_KIRK; i <= OBJECT_REDSHIRT; i++) {
				Common::String anim = getCrewmanAnimFilename(i, "getd");
				anim += directions[i][_vm->_roomIndex];
				loadActorAnim2(i, anim);
			}
			showDescription(TX_MUD0N006);
			showGameOverMenu();
		}
	}
}

}
