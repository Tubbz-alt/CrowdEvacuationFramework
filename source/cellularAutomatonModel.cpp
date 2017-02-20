#include "cellularAutomatonModel.h"

CellularAutomatonModel::CellularAutomatonModel() {
	mFloorField.read("./data/config_floorField.txt");
	mAgentManager.read("./data/config_agent.txt");

	mFloorField.update();
	mFloorField.print();

	mIsOccupied = new bool*[mFloorField.mFloorFieldDim[1]];
	for (int i = 0; i < mFloorField.mFloorFieldDim[1]; i++)
		mIsOccupied[i] = new bool[mFloorField.mFloorFieldDim[0]];
	setCellOccupancyState();

	mNumTimesteps = 0;
	mRNG.seed(time(0));
}

void CellularAutomatonModel::update() {
	if (countAgentsHavingLeft() == mAgentManager.mNumAgents)
		return;

	boost::random::uniform_real_distribution<> distribution(0.0, 1.0);
	array2i *tmpAgents = new array2i[mAgentManager.mNumAgents]; // cell an agent will move to
	
	for (int i = 0; i < mAgentManager.mNumAgents; i++) {
		if (!mAgentManager.mIsVisible[i])
			continue;

		/*
		 * Check whether the agent arrives at any exit.
		 */
		for (int j = 0; j < mFloorField.mNumExits; j++) {
			if (mFloorField.mIsVisible_Exit[j] && mAgentManager.mAgents[i] == mFloorField.mExits[j]) {
				mAgentManager.mIsVisible[i] = false;
				mIsOccupied[mAgentManager.mAgents[i][1]][mAgentManager.mAgents[i][0]] = false;
			}
		}
		if (!mAgentManager.mIsVisible[i]) // the agent arives at an exit indeed
			continue;

		/*
		 * Handle agent movement.
		 */
		array2i curCoord = mAgentManager.mAgents[i];

		if (distribution(mRNG) < mAgentManager.mPanicProb) { // agent in panic does not move
			tmpAgents[i] = curCoord;
			cout << "Agent " << i << " is in panic (Position: " << curCoord << ")" << endl;
		}
		else {
			/*
			 * Find available cells with the lowest cell value.
			 */
			double lowestCellValue = mFloorField.mCells[curCoord[1]][curCoord[0]];
			boost::container::small_vector<array2i, 9> possibleCoords;

			// right cell
			if (curCoord[0] + 1 < mFloorField.mFloorFieldDim[0] && !mIsOccupied[curCoord[1]][curCoord[0] + 1]) {
				if (lowestCellValue == mFloorField.mCells[curCoord[1]][curCoord[0] + 1])
					possibleCoords.push_back(array2i{ { curCoord[0] + 1, curCoord[1] } });
				else if (lowestCellValue > mFloorField.mCells[curCoord[1]][curCoord[0] + 1]) {
					lowestCellValue = mFloorField.mCells[curCoord[1]][curCoord[0] + 1];
					possibleCoords.clear();
					possibleCoords.push_back(array2i{ { curCoord[0] + 1, curCoord[1] } });
				}
			}

			// left cell
			if (curCoord[0] - 1 >= 0 && !mIsOccupied[curCoord[1]][curCoord[0] - 1]) {
				if (lowestCellValue == mFloorField.mCells[curCoord[1]][curCoord[0] - 1])
					possibleCoords.push_back(array2i{ { curCoord[0] - 1, curCoord[1] } });
				else if (lowestCellValue > mFloorField.mCells[curCoord[1]][curCoord[0] - 1]) {
					lowestCellValue = mFloorField.mCells[curCoord[1]][curCoord[0] - 1];
					possibleCoords.clear();
					possibleCoords.push_back(array2i{ { curCoord[0] - 1, curCoord[1] } });
				}
			}

			// up cell
			if (curCoord[1] + 1 < mFloorField.mFloorFieldDim[1] && !mIsOccupied[curCoord[1] + 1][curCoord[0]]) {
				if (lowestCellValue == mFloorField.mCells[curCoord[1] + 1][curCoord[0]])
					possibleCoords.push_back(array2i{ { curCoord[0], curCoord[1] + 1 } });
				else if (lowestCellValue > mFloorField.mCells[curCoord[1] + 1][curCoord[0]]) {
					lowestCellValue = mFloorField.mCells[curCoord[1] + 1][curCoord[0]];
					possibleCoords.clear();
					possibleCoords.push_back(array2i{ { curCoord[0], curCoord[1] + 1 } });
				}
			}

			// down cell
			if (curCoord[1] - 1 >= 0 && !mIsOccupied[curCoord[1] - 1][curCoord[0]]) {
				if (lowestCellValue == mFloorField.mCells[curCoord[1] - 1][curCoord[0]])
					possibleCoords.push_back(array2i{ { curCoord[0], curCoord[1] - 1 } });
				else if (lowestCellValue > mFloorField.mCells[curCoord[1] - 1][curCoord[0]]) {
					lowestCellValue = mFloorField.mCells[curCoord[1] - 1][curCoord[0]];
					possibleCoords.clear();
					possibleCoords.push_back(array2i{ { curCoord[0], curCoord[1] - 1 } });
				}
			}

			// upper right cell
			if (curCoord[0] + 1 < mFloorField.mFloorFieldDim[0] && curCoord[1] + 1 < mFloorField.mFloorFieldDim[1] && !mIsOccupied[curCoord[1] + 1][curCoord[0] + 1]) {
				if (lowestCellValue == mFloorField.mCells[curCoord[1] + 1][curCoord[0] + 1])
					possibleCoords.push_back(array2i{ { curCoord[0] + 1, curCoord[1] + 1 } });
				else if (lowestCellValue > mFloorField.mCells[curCoord[1] + 1][curCoord[0] + 1]) {
					lowestCellValue = mFloorField.mCells[curCoord[1] + 1][curCoord[0] + 1];
					possibleCoords.clear();
					possibleCoords.push_back(array2i{ { curCoord[0] + 1, curCoord[1] + 1 } });
				}
			}

			// lower left cell
			if (curCoord[0] - 1 >= 0 && curCoord[1] - 1 >= 0 && !mIsOccupied[curCoord[1] - 1][curCoord[0] - 1]) {
				if (lowestCellValue == mFloorField.mCells[curCoord[1] - 1][curCoord[0] - 1])
					possibleCoords.push_back(array2i{ { curCoord[0] - 1, curCoord[1] - 1 } });
				else if (lowestCellValue > mFloorField.mCells[curCoord[1] - 1][curCoord[0] - 1]) {
					lowestCellValue = mFloorField.mCells[curCoord[1] - 1][curCoord[0] - 1];
					possibleCoords.clear();
					possibleCoords.push_back(array2i{ { curCoord[0] - 1, curCoord[1] - 1 } });
				}
			}
			
			// lower right cell
			if (curCoord[0] + 1 < mFloorField.mFloorFieldDim[0] && curCoord[1] - 1 >= 0 && !mIsOccupied[curCoord[1] - 1][curCoord[0] + 1]) {
				if (lowestCellValue == mFloorField.mCells[curCoord[1] - 1][curCoord[0] + 1])
					possibleCoords.push_back(array2i{ { curCoord[0] + 1, curCoord[1] - 1 } });
				else if (lowestCellValue > mFloorField.mCells[curCoord[1] - 1][curCoord[0] + 1]) {
					lowestCellValue = mFloorField.mCells[curCoord[1] - 1][curCoord[0] + 1];
					possibleCoords.clear();
					possibleCoords.push_back(array2i{ { curCoord[0] + 1, curCoord[1] - 1 } });
				}
			}
			
			// upper left cell
			if (curCoord[0] - 1 >= 0 && curCoord[1] + 1 < mFloorField.mFloorFieldDim[1] && !mIsOccupied[curCoord[1] + 1][curCoord[0] - 1]) {
				if (lowestCellValue == mFloorField.mCells[curCoord[1] + 1][curCoord[0] - 1])
					possibleCoords.push_back(array2i{ { curCoord[0] - 1, curCoord[1] + 1 } });
				else if (lowestCellValue > mFloorField.mCells[curCoord[1] + 1][curCoord[0] - 1]) {
					lowestCellValue = mFloorField.mCells[curCoord[1] + 1][curCoord[0] - 1];
					possibleCoords.clear();
					possibleCoords.push_back(array2i{ { curCoord[0] - 1, curCoord[1] + 1 } });
				}
			}

			/*
			 * Decide the cell where the agent will intend to move.
			 */
			if (possibleCoords.size() == 0)
				tmpAgents[i] = curCoord;
			else
				tmpAgents[i] = possibleCoords[(int)floor(distribution(mRNG) * possibleCoords.size())];
		}
	}

	/*
	 * Handle agent interaction.
	 */
	bool *isProcessed = new bool[mAgentManager.mNumAgents];
	std::fill_n(isProcessed, mAgentManager.mNumAgents, false);

	for (int i = 0; i < mAgentManager.mNumAgents; i++) {
		if (!mAgentManager.mIsVisible[i] || isProcessed[i])
			continue;

		boost::container::small_vector<int, 9> agentsInConflict;
		agentsInConflict.push_back(i);
		isProcessed[i] = true;

		for (int j = 0; j < mAgentManager.mNumAgents; j++) {
			if (!mAgentManager.mIsVisible[j] || isProcessed[j])
				continue;

			if (tmpAgents[i] == tmpAgents[j]) {
				agentsInConflict.push_back(j);
				isProcessed[j] = true;
			}
		}

		// move the agent
		int winner = agentsInConflict[(int)floor(distribution(mRNG) * agentsInConflict.size())];
		mIsOccupied[mAgentManager.mAgents[winner][1]][mAgentManager.mAgents[winner][0]] = false;
		mAgentManager.mAgents[winner] = tmpAgents[winner];
		mIsOccupied[mAgentManager.mAgents[winner][1]][mAgentManager.mAgents[winner][0]] = true;
	}

	mNumTimesteps++;

	cout << "Timestep " << mNumTimesteps << ": " << mAgentManager.mNumAgents - countAgentsHavingLeft() << " agent(s) having not left" << endl;

	delete[] isProcessed;
	delete[] tmpAgents;
}

void CellularAutomatonModel::editAgents(array2f worldCoord) {
	array2i coord{ { (int)floor(worldCoord[0] / mFloorField.mCellSize[0]), (int)floor(worldCoord[1] / mFloorField.mCellSize[1]) } }; // coordinate in the floor field

	if (coord[0] < 0 || coord[0] >= mFloorField.mFloorFieldDim[0] || coord[1] < 0 || coord[1] >= mFloorField.mFloorFieldDim[1])
		return;
	// only cells which are not occupied by exits or obstacles can be edited
	else if (mFloorField.isExisting_Exit(coord) == -1 && mFloorField.isExisting_Obstacle(coord) == -1) {
		mAgentManager.edit(coord);
		setCellOccupancyState();
	}
}

void CellularAutomatonModel::editExits(array2f worldCoord) {
	array2i coord{ { (int)floor(worldCoord[0] / mFloorField.mCellSize[0]), (int)floor(worldCoord[1] / mFloorField.mCellSize[1]) } }; // coordinate in the floor field

	if (coord[0] < 0 || coord[0] >= mFloorField.mFloorFieldDim[0] || coord[1] < 0 || coord[1] >= mFloorField.mFloorFieldDim[1])
		return;
	// only cells which are not occupied by agents or obstacles can be edited
	else if (mAgentManager.isExisting(coord) == -1 && mFloorField.isExisting_Obstacle(coord) == -1) {
		mFloorField.editExits(coord);
		setCellOccupancyState();
	}
}

void CellularAutomatonModel::editObstacles(array2f worldCoord) {
	array2i coord{ { (int)floor(worldCoord[0] / mFloorField.mCellSize[0]), (int)floor(worldCoord[1] / mFloorField.mCellSize[1]) } }; // coordinate in the floor field

	if (coord[0] < 0 || coord[0] >= mFloorField.mFloorFieldDim[0] || coord[1] < 0 || coord[1] >= mFloorField.mFloorFieldDim[1])
		return;
	// only cells which are not occupied by agents or exits can be edited
	else if (mAgentManager.isExisting(coord) == -1 && mFloorField.isExisting_Exit(coord) == -1) {
		mFloorField.editObstacles(coord);
		setCellOccupancyState();
	}
}

void CellularAutomatonModel::refreshTimer() {
	mNumTimesteps = 0;
}

void CellularAutomatonModel::save() {
	mFloorField.save();
	mAgentManager.save();
}

void CellularAutomatonModel::draw() {
	mFloorField.draw();
	mAgentManager.draw();
}

void CellularAutomatonModel::setCellOccupancyState() {
	// initialize
	for (int i = 0; i < mFloorField.mFloorFieldDim[1]; i++)
		std::fill_n(mIsOccupied[i], mFloorField.mFloorFieldDim[0], false);

	// cell occupied by an obstacle
	for (int i = 0; i < mFloorField.mNumObstacles; i++) {
		if (mFloorField.mIsVisible_Obstacle[i])
			mIsOccupied[mFloorField.mObstacles[i][1]][mFloorField.mObstacles[i][0]] = true;
	}

	// cell occupied by an agent
	for (int i = 0; i < mAgentManager.mNumAgents; i++) {
		if (mAgentManager.mIsVisible[i])
			mIsOccupied[mAgentManager.mAgents[i][1]][mAgentManager.mAgents[i][0]] = true;
	}
}

int CellularAutomatonModel::countAgentsHavingLeft() {
	int num = 0;
	for (int i = 0; i < mAgentManager.mNumAgents; i++) {
		if (!mAgentManager.mIsVisible[i])
			num++;
	}
	return num;
}