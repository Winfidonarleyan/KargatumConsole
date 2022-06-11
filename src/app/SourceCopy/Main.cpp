/*
 * This file is part of the WarheadApp Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "CopyMgr.h"
#include "Log.h"
#include "GitRevision.h"

int main()
{
    LOG_INFO("> {}", GitRevision::GetFullVersion());
    LOG_INFO("--");

    if (!sCopyMgr->Load())
        return 1;

    sCopyMgr->SendBaseInfo();

    return 0;
}
