#include <stdafx.h>

#include <QCoreApplication>
#include <QDebug>
#if defined (_CONSOLE)
#include <iostream>
#else
#include <QMessageBox>
#endif
#include <QSettings>
#include "resource.h"
#include "FrameList.h"
#include "ImageListModel.h"
#include "RegisterEngine.h"
#include "Workspace.h"
#include <direct.h>

#include "ZExcept.h"

namespace {

	bool parseLine(QString line, int& lChecked, QString& strType, QString& strFile)
	{
		bool result = false;

		//
		// trailing \n has been removed by .trimmed() before calling this mf
		//
		auto list = line.split(QLatin1Char('\t'));

		//
		// line should now be split into three sections each in its own entry in the QStringList
		// list = [ "checked", "Type", "File" ] 
		//
		if (list.size() == 3)
		{
			lChecked = list[0].toInt();
			strType = list[1];
			strFile = list[2];
			result = true;
		}
		return result;
	}

	bool isChangeGroupLine(QString line, int& groupId, QString& groupName)
	{
		bool bResult = false;

		if (line.left(9) == "#GROUPID#")
		{
			line.remove(0, 9);	// Strip off the identifier

			auto list = line.split(QLatin1Char('\t'));
			//
			// line should now be split into one or two sections each in its own entry in the QStringList
			// list = [ "groupId", "groupName"] 
			//

			groupId = list[0].toInt();
			if (2 == list.size())
				groupName = list[1];

			bResult = true;
		}

		return bResult;
	}

} // namespace

/* ------------------------------------------------------------------- */

namespace DSS
{
	size_t FrameList::checkedImageCount(const PICTURETYPE type, const int id) const
	{
		size_t result = 0;

		// Iterate over all groups.
		for (int i = 0; i != imageGroups.size(); ++i)
		{
			// If the group number passed in was -1 then want to count the number of
			// checked images of the relevant type in ALL groups.  Otherwise only
			// count checked images for the passed group number.
			if (-1 == id || id == i)
			{
				for (auto it = imageGroups[i].pictures->cbegin(); it != imageGroups[i].pictures->cend(); ++it)
				{
					if (it->m_PictureType == type && it->m_bChecked == Qt::Checked)
						++result;
				}
			}
		}

		return result;
	}

	size_t FrameList::countUnregisteredCheckedLightFrames(const int id) const
	{
		size_t result = 0;

		// Iterate over all groups
		for (const auto& group : imageGroups)
		{
			// If the group number passed in was -1 then want to count the number of
			// checked images of the relevant type in ALL groups.  Otherwise only
			// count checked images for the passed group number.
			if (-1 == id || id == group.index())
			{
				for (auto it = group.pictures->cbegin(); it != group.pictures->cend(); ++it)
				{
					if (it->IsLightFrame() && it->m_bChecked == Qt::Checked && !it->m_bRegistered)
						++result;
				}
			}
		}
		return result;
	}



	void FrameList::clearOffsets()
	{
		for (auto& group : imageGroups)
		{
			int row = 0;

			for (auto it = group.pictures->begin(); it != group.pictures->end(); ++it)
			{
				if (it->IsLightFrame())
					it->m_bDeltaComputed = false;
				if (index == group.index())
				{
					//
					// Tell the table view which columns have been impacted
					//
					imageGroups[index].pictures->emitChanged(row, row,
						static_cast<int>(Column::dX),
						static_cast<int>(Column::Angle));
				}
				++row;
			}
		}
		//
		// Tell the table view which columns have been impacted
		//
		imageGroups[index].pictures->emitChanged(0, imageGroups[index].pictures->rowCount(),
			static_cast<int>(Column::dX),
			static_cast<int>(Column::Angle));
	}

	void FrameList::clearOffset(fs::path file)
	{
		const int group = Group::whichGroupContains(file);

		ZASSERTSTATE(-1 != group);
		int row = 0;
		for (auto it = imageGroups[group].pictures->begin(); it != imageGroups[group].pictures->end(); ++it)
		{
			if (file == it->filePath)
			{
				it->m_bDeltaComputed = false;
				//
				// Ask the Table Model to tell the table view which rows/columns have changed
				//
				if (index == group)
				{
					imageGroups[group].pictures->emitChanged(row, row,
						static_cast<int>(Column::dX),
						static_cast<int>(Column::Angle));
				}
				return;
			}
			++row;
		}
	}

	void FrameList::updateOffset(fs::path file, double xOffset, double yOffset, double angle, const CBilinearParameters& transform, const VOTINGPAIRVECTOR& vVotedPairs)
	{
		const int group = Group::whichGroupContains(file);

		ZASSERTSTATE(-1 != group);
		int row = 0;
		for (auto it = imageGroups[group].pictures->begin(); it != imageGroups[group].pictures->end(); ++it)
		{
			if (file == it->filePath)
			{
				it->m_bDeltaComputed = true;
				imageGroups[group].pictures->setData(row, Column::dX, xOffset);
				imageGroups[group].pictures->setData(row, Column::dY, yOffset);
				imageGroups[group].pictures->setData(row, Column::Angle, angle);
				it->m_Transformation = transform;
				it->m_vVotedPairs = vVotedPairs;

				return;
			}
			++row;
		}
	}

	QString FrameList::getReferenceFrame() const
	{
		for (const auto& group : imageGroups)
		{
			for (auto it = group.pictures->cbegin(); it != group.pictures->cend(); ++it)
			{
				if (it->IsLightFrame() && it->m_bChecked == Qt::Checked && it->m_bUseAsStarting)
				{
					return QString::fromStdU16String(it->filePath.generic_u16string());
				}
			}
		}
		return QString();
	}
	
	bool FrameList::getReferenceFrame(CString& string) const
	{
		bool result = false;
		for (const auto& group : imageGroups)
		{
			for (auto it = group.pictures->cbegin(); it != group.pictures->cend(); ++it)
			{
				if (it->IsLightFrame() && it->m_bChecked == Qt::Checked && it->m_bUseAsStarting)
				{
					result = true;
					string = it->filePath.generic_wstring().c_str();
					return result;
				}
			}
		}
		return result;
	}

	QString FrameList::getFirstCheckedLightFrame() const
	{
		for (const auto& group : imageGroups)
		{
			for (auto it = group.pictures->cbegin(); it != group.pictures->cend(); ++it)
			{
				if (it->IsLightFrame() && it->m_bChecked == Qt::Checked)
				{
					return QString::fromStdU16String(it->filePath.generic_u16string());
				}
			}
		}

		return QString();
	}

	void FrameList::fillTasks(CAllStackingTasks& tasks)
	{
		size_t				comets = 0;
		bool				bReferenceFrameHasComet = false;
		bool				bReferenceFrameSet = false;
		double				fMaxScore = -1.0;

		// Iterate over all groups.
		for (std::uint32_t group = 0; group != imageGroups.size(); ++group)
		{
			// and then over each image in the group
			for (auto it = imageGroups[group].pictures->cbegin(); it != imageGroups[group].pictures->cend(); ++it)
			{
				if (it->m_bChecked == Qt::Checked)
				{
					if (it->m_bUseAsStarting)
					{
						bReferenceFrameSet = true;
						bReferenceFrameHasComet = it->m_bComet;
					}
					if (!bReferenceFrameSet && (it->m_fOverallQuality > fMaxScore))
					{
						fMaxScore = it->m_fOverallQuality;
						bReferenceFrameHasComet = it->m_bComet;
					}
					tasks.AddFileToTask(*it, group);
					if (it->m_bComet)
						comets++;
				}
			}
		}

		if (comets > 1 && bReferenceFrameHasComet)
			tasks.SetCometAvailable(true);
		tasks.ResolveTasks();
	}

	/* ------------------------------------------------------------------- */

	FrameList& FrameList::saveListToFile(fs::path file)
	{
		if (std::FILE* hFile =
#if defined(_WINDOWS)
			_wfopen(file.c_str(), L"wt")
#else
			std::fopen(file.c_ctr(), "wt")
#endif
			)
		{
			const fs::path directory = file.has_parent_path() ? file.parent_path() : file.root_path();

			fprintf(hFile, "DSS file list\n");
			fprintf(hFile, "CHECKED\tTYPE\tFILE\n");

			decltype(ListBitMap::m_groupId) groupId = 0;

			for (auto &g : imageGroups)
			{
				// and then over each image in the group
				for (auto it = g.pictures->cbegin(); it != g.pictures->cend(); ++it) 
				{
					int checked = 0;
					QString type;

					if (groupId != it->m_groupId)
					{
						groupId = it->m_groupId;
						fprintf(hFile, "#GROUPID#%hu\t%s\n", groupId, g.name().toUtf8().constData());
					}
					checked = it->m_bChecked == Qt::Checked ? 1 : 0;
					if (it->IsLightFrame())
					{
						if (it->m_bUseAsStarting)
							type = "reflight";
						else
							type = "light";
					}
					else if (it->IsDarkFrame())
						type = "dark";
					else if (it->IsDarkFlatFrame())
						type = "darkflat";
					else if (it->IsOffsetFrame())
						type = "offset";
					else if (it->IsFlatFrame())
						type = "flat";

					//
					// Convert the path to the file to one that is relative to
					// the directory containing the file list file of that's possible.
					// If not just leave it as the absolute path.
					//
					fs::path path{ it->filePath.lexically_proximate(directory) };
#pragma warning (suppress:4477)
					fprintf(hFile, "%ld\t%s\t%s\n", checked,
						type.toUtf8().constData(),
						path.generic_u8string().c_str());
				}
				g.setDirty(false);
			}

			//
			// Save workspace settings
			//
			Workspace workspace;
			workspace.SaveToFile(hFile);
			workspace.resetDirty();

			fclose(hFile);
		}
		return *this;
	}

	/* ------------------------------------------------------------------- */

	FrameList& FrameList::loadFilesFromList(fs::path fileList)
	{
		int groupId = 0;
		std::error_code ec;

		//
		// Remember current directory and extract directory containing filelist
		//
		const fs::path directory = fileList.has_parent_path() ? fileList.parent_path() : fileList.root_path();
		const fs::path oldCWD = fs::current_path(ec); // Save CWD

		if (ec)
		{
			ZTRACE_RUNTIME("fs::current_path() failed with error code %ld, %s",
				ec.value(), ec.message().c_str());
		}
		
		if (std::FILE* hFile =
#if defined(_WINDOWS)
			_wfopen(fileList.c_str(), L"rt")
#else
			std::fopen(fileList.c_ctr(), "rt")
#endif
			)
		{
			CHAR			szBuffer[2000];
			QString			strValue;
			bool			bContinue = false;

			fs::current_path(directory, ec);				// Set CWD to fileList dir
			if (!ec)
				bContinue = true;
			else
			{
				ZTRACE_RUNTIME("fs::current_path(%s) failed with error code %ld, %s",
					directory.generic_string().c_str(), ec.value(), ec.message().c_str());
				bContinue = false;
			}

			// Read scan line
			if (fgets(szBuffer, sizeof(szBuffer), hFile))
			{
				strValue = QString::fromUtf8(szBuffer);
				if (!strValue.compare("DSS file list\n", Qt::CaseInsensitive))
					bContinue = true;
			}

			if (bContinue)
			{
				bContinue = false;
				if (fgets(szBuffer, sizeof(szBuffer), hFile))
				{
					strValue = QString::fromUtf8(szBuffer);
					if (!strValue.compare("CHECKED\tTYPE\tFILE\n", Qt::CaseInsensitive))
						bContinue = true;
				}
			};

			if (bContinue)
			{
				// Read the file info
				Workspace			workspace;
				CHAR				szLine[10000];

				while (fgets(szLine, sizeof(szLine), hFile))
				{
					int checkState = Qt::Unchecked;
					QString			strType;
					QString			strFile;
					QString			strGroupName;
					QString			strLine{ QString::fromUtf8(szLine).trimmed() };

					bool			bUseAsStarting = false;

					if (workspace.ReadFromString(strLine))
					{
					}
					else if (isChangeGroupLine(strLine, groupId, strGroupName))
					{
						//
						// Zero index groupId must be same as count of groups when adding
						// a group
						//
						if (groupId == imageGroups.size())
							addGroup();
						ZASSERTSTATE(groupId < (1 + imageGroups.size()));

						setGroup(groupId);	// Select the group in question
						if (!strGroupName.isEmpty())
							imageGroups[groupId].setName(strGroupName);
					}
					else if (parseLine(strLine, checkState, strType, strFile))
					{
						PICTURETYPE		Type = PICTURETYPE_UNKNOWN;

						if ("light" == strType)
							Type = PICTURETYPE_LIGHTFRAME;
						else if ("dark" == strType)
							Type = PICTURETYPE_DARKFRAME;
						else if ("darkflat" == strType)
							Type = PICTURETYPE_DARKFLATFRAME;
						else if ("flat" == strType)
							Type = PICTURETYPE_FLATFRAME;
						else if ("offset" == strType)
							Type = PICTURETYPE_OFFSETFRAME;
						else if ("reflight" == strType)
						{
							Type = PICTURETYPE_REFLIGHTFRAME;
							bUseAsStarting = true;
						}

						if (Type != PICTURETYPE_UNKNOWN)
						{
							//
							// Convert relative path to absolute path. 
							//
							fs::path filePath{ strFile.toStdU16String() };

							// If it's not already an absolute path
							if (!filePath.is_absolute())
							{
								filePath = fs::absolute(filePath, ec);
							}

							if (ec)
							{
								ZTRACE_RUNTIME("fs::absolute(%s) failed with error code %ld, %s",
									filePath.generic_u8string().c_str(), ec.value(), ec.message().c_str());
							}

							// Check that the file exists and that it hasn't already been loaded
							if (is_regular_file(filePath))
							{
								//
								// Check all groups to see if this file has already been loaded
								//
								if (groupId = Group::whichGroupContains(filePath); groupId >= 0)
								{
									//
									// If the file has already been loaded complain
									//
									QString errorMessage(
										QCoreApplication::translate("DSS::StackingDlg", "File %1 was not loaded because it was already loaded in group %2 (%3)")
										.arg(filePath.generic_string().c_str())
										.arg(groupId)
										.arg(groupName(groupId)));

#if defined(_CONSOLE)
									std::cerr << errorMessage.toUtf8().constData();
#else
									QMessageBox::warning(nullptr, "DeepSkyStacker", errorMessage, QMessageBox::Ok);
#endif
									return *this;
								}
								else
								{
									beginInsertRows(1);
									addFile(filePath, Type, (checkState == 1));
									endInsertRows();
								}
							}
						}
					}
				}
				workspace.resetDirty();
			}
			fclose(hFile);
		}

		fs::current_path(oldCWD);
		setDirty(false);
		return *this;
	}

	void FrameList::blankCheckedItemScores() const
	{
		// Iterate over all groups.
		for (uint16_t group = 0; group != imageGroups.size(); ++group)
		{
			// and then over each image in the group
			for (auto it = imageGroups[group].pictures->begin();
				it != imageGroups[group].pictures->end(); ++it)
			{
				if (it->m_bChecked == Qt::Checked && it->IsLightFrame())
				{
					it->m_bRegistered = false;
				}
			}
		}
	}

	bool FrameList::areCheckedImagesCompatible(QString& reason) const
	{
		bool				bResult = true;
		bool				bFirst = true;
		const ListBitMap* lb{ nullptr };

		// Iterate over all groups.
		for (uint16_t group = 0; group != imageGroups.size() && true == bResult; ++group)
		{
			// and then over each image in the group
			for (auto it = imageGroups[group].pictures->cbegin();
				it != imageGroups[group].pictures->cend(); ++it)
			{
				if (it->m_bChecked == Qt::Checked)
				{
					if (bFirst)
					{
						lb = &(*it);
						bFirst = false;
					}
					else
					{
						bResult = lb->IsCompatible(*it);
						if (false == bResult)
						{
							reason = lb->incompatibilityReason;
							break;
						}
					}
				}
			}
		}

		return bResult;
	}

	/* ------------------------------------------------------------------- */


	void FrameList::updateCheckedItemScores()
	{
		// Iterate over all groups.
		for (uint16_t group = 0; group != imageGroups.size(); ++group)
		{
			// and then over each image in the group
			int row = 0;
			for (auto it = imageGroups[group].pictures->begin();
				it != imageGroups[group].pictures->end(); ++it, ++row)
			{
				if (it->m_bChecked == Qt::Checked &&
					it->IsLightFrame())
				{
					CLightFrameInfo		bmpInfo;

					bmpInfo.SetBitmap(it->filePath, false, false);

					//
					// Update list information, but beware that you must use setData() for any of the columns
					// that are defined in the Column enumeration as they are used for the 
					// QTableView.   If this isn't done, the image list view won't get updated.
					//
					// The "Sky Background" (Column::BackgroundCol) is a special case it's a class, not a primitive, so the model 
					// class has a specific member function to set that.
					//
					// Other member of ListBitMap (e.g.) m_bRegistered and m_bComet can be updated directly.
					//
					if (bmpInfo.m_bInfoOk)
					{
						it->m_bRegistered = true;
						imageGroups[group].pictures->setData(row, Column::Score, bmpInfo.m_fOverallQuality);
						imageGroups[group].pictures->setData(row, Column::FWHM, bmpInfo.m_fFWHM);
						it->m_bComet = bmpInfo.m_bComet;		// MUST Set this Before updating Column::Stars
						imageGroups[group].pictures->setData(row, Column::Stars, (int)bmpInfo.m_vStars.size());
						imageGroups[group].pictures->setData(row, Column::Background, (uint32_t)bmpInfo.m_vStars.size());
						imageGroups[group].pictures->setSkyBackground(row, bmpInfo.m_SkyBackground);

					}
					else
					{
						it->m_bRegistered = false;
					}
				}
			}
		}
	}

	void FrameList::updateItemScores(const QString& fileName)
	{
		int row = 0;

		for (auto it = imageGroups[index].pictures->begin();
			it != imageGroups[index].pictures->end(); ++it)
		{
			if (it->filePath == fs::path(fileName.toStdString()) && it->IsLightFrame())
			{
				CLightFrameInfo		bmpInfo;

				bmpInfo.SetBitmap(it->filePath, false, false);

				//
				// Update list information, but beware that you must use setData() for any of the columns
				// that are defined in the Column enumeration as they are used for the 
				// QTableView.   If this isn't done, the image list view won't get updated.
				//
				// The "Sky Background" (Column::BackgroundCol) is a special case it's a class, not a primitive, so the model 
				// class has a specific member function to set that.
				//
				// Other member of ListBitMap (e.g.) m_bRegistered and m_bComet can be updated directly.
				//
				if (bmpInfo.m_bInfoOk)
				{
					it->m_bRegistered = true;
					imageGroups[index].pictures->setData(row, Column::Score, bmpInfo.m_fOverallQuality);
					imageGroups[index].pictures->setData(row, Column::FWHM, bmpInfo.m_fFWHM);
					it->m_bComet = bmpInfo.m_bComet;		// MUST Set this Before updating Column::Stars
					imageGroups[index].pictures->setData(row, Column::Stars, (int)bmpInfo.m_vStars.size());
					imageGroups[index].pictures->setData(row, Column::Background, (uint32_t)bmpInfo.m_vStars.size());
					imageGroups[index].pictures->setSkyBackground(row, bmpInfo.m_SkyBackground);

				}
				else
				{
					it->m_bRegistered = false;
				}
			}
			++row;
		}
	}

	void FrameList::checkAll(bool check)
	{
		for (int id = 0; id != imageGroups.size(); ++id)
		{
			auto& group = imageGroups[id];
			for (int32_t idx = 0; idx < group.pictures->mydata.size(); ++idx)
			{
				auto& file = group.pictures->mydata[idx];
				if (check) file.m_bChecked = Qt::Checked;
				else file.m_bChecked = Qt::Unchecked;
			}
			QModelIndex start{ group.pictures->createIndex(0, 0) };
			QModelIndex end{ group.pictures->createIndex(group.pictures->rowCount(), 0) };
			QVector<int> role{ Qt::CheckStateRole };
			group.pictures->dataChanged(start, end, role);
			group.setDirty();
		}
	}

	void FrameList::checkAllDarks(bool check)
	{
		for (int id = 0; id != imageGroups.size(); ++id)
		{
			auto& group = imageGroups[id];
			for (int32_t idx = 0; idx < group.pictures->mydata.size(); ++idx)
			{
				auto& file = group.pictures->mydata[idx];
				if (file.IsDarkFrame())
				{
					if (check) file.m_bChecked = Qt::Checked;
					else file.m_bChecked = Qt::Unchecked;
				}
				QModelIndex start{ group.pictures->createIndex(index, 0) };
				QModelIndex end{ group.pictures->createIndex(index, 0) };
				QVector<int> role{ Qt::CheckStateRole };
				group.pictures->dataChanged(start, end, role);
			}

			group.setDirty();
		}
	}
	/* ------------------------------------------------------------------- */

	void FrameList::checkAllFlats(bool check)
	{
		for (int id = 0; id != imageGroups.size(); ++id)
		{
			auto& group = imageGroups[id];
			for (int32_t idx = 0; idx < group.pictures->mydata.size(); ++idx)
			{
				auto& file = group.pictures->mydata[idx];
				if (file.IsFlatFrame())
				{
					if (check) file.m_bChecked = Qt::Checked;
					else file.m_bChecked = Qt::Unchecked;
				}
				QModelIndex start{ group.pictures->createIndex(index, 0) };
				QModelIndex end{ group.pictures->createIndex(index, 0) };
				QVector<int> role{ Qt::CheckStateRole };
				group.pictures->dataChanged(start, end, role);
			}

			group.setDirty();
		}
	}

	/* ------------------------------------------------------------------- */

	void FrameList::checkAllOffsets(bool check)
	{
		for (int id = 0; id != imageGroups.size(); ++id)
		{
			auto& group = imageGroups[id];
			for (int32_t idx = 0; idx < group.pictures->mydata.size(); ++idx)
			{
				auto& file = group.pictures->mydata[idx];
				if (file.IsOffsetFrame())
				{
					if (check) file.m_bChecked = Qt::Checked;
					else file.m_bChecked = Qt::Unchecked;
				}
				QModelIndex start{ group.pictures->createIndex(index, 0) };
				QModelIndex end{ group.pictures->createIndex(index, 0) };
				QVector<int> role{ Qt::CheckStateRole };
				group.pictures->dataChanged(start, end, role);
			}

			group.setDirty();
		}
	}

	/* ------------------------------------------------------------------- */

	void FrameList::checkAllLights(bool check)
	{
		for (int id = 0; id != imageGroups.size(); ++id)
		{
			auto& group = imageGroups[id];
			for (int32_t idx = 0; idx < group.pictures->mydata.size(); ++idx)
			{
				auto& file = group.pictures->mydata[idx];
				if (file.IsLightFrame())
				{
					if (check) file.m_bChecked = Qt::Checked;
					else file.m_bChecked = Qt::Unchecked;
					QModelIndex start{ group.pictures->createIndex(idx, 0) };
					QModelIndex end{ group.pictures->createIndex(idx, 0) };
					QVector<int> role{ Qt::CheckStateRole };
					group.pictures->dataChanged(start, end, role);
				}
			}
			group.setDirty();
		}
	}

	void FrameList::checkImage(const QString & image, bool check)
	{
		for (int id = 0; id != imageGroups.size(); ++id)
		{
			auto& group = imageGroups[id];
			for (int32_t idx = 0; idx < group.pictures->mydata.size(); ++idx)
			{
				auto& file = group.pictures->mydata[idx];
				if (image == file.m_strFile && file.IsLightFrame())
				{
					if (check) file.m_bChecked = Qt::Checked;
					else file.m_bChecked = Qt::Unchecked;
					group.setDirty();
					QModelIndex start{ group.pictures->createIndex(idx, 0) };
					QModelIndex end{ group.pictures->createIndex(idx, 0) };
					QVector<int> role{ Qt::CheckStateRole };
					group.pictures->dataChanged(start, end, role);
					return;
				}
			}
		}
	}
	/* ------------------------------------------------------------------- */


	void FrameList::checkAbove(double threshold)
	{
		for (int id = 0; id != imageGroups.size(); ++id)
		{
			auto& group = imageGroups[id];
			for (int idx = 0; idx != group.pictures->mydata.size(); ++idx)
			{
				auto& file = group.pictures->mydata[idx];
				if (file.IsLightFrame())
				{
					file.m_bChecked = 
						(file.m_fOverallQuality >= threshold) ? Qt::Checked : Qt::Unchecked;
					QModelIndex start{ group.pictures->createIndex(idx, 0) };
					QModelIndex end{ group.pictures->createIndex(idx, 0) };
					QVector<int> role{ Qt::CheckStateRole };
					group.pictures->dataChanged(start, end, role);
				}
			}
			group.setDirty();
		}
	}

	/* ------------------------------------------------------------------- */

	void FrameList::changePictureType([[maybe_unused]] int nItem, [[maybe_unused]]PICTURETYPE PictureType)
	{
		qDebug() << "In " <<
#ifdef __FUNCSIG__
			__FUNCSIG__;
#elif defined(__PRETTY_FUNCTION__)
			__PRETTY_FUNCTION__;
#else
			__FUNCTION__;
#endif			
		//LONG			lIndice;  TODO
		//CString			strFileName;

		//strFileName = m_vFiles[mItem].m_strFileName;
		//virtual bool addFile(fs::path file, PICTURETYPE PictureType = PICTURETYPE_LIGHTFRAME, bool bCheck = false, int nItem = -1)
		//addFile(strFileName, m_dwCurrentGroupID, m_dwCurrentJobID, PictureType, FALSE, nItem);
	}

	/* ------------------------------------------------------------------- */

	void FrameList::checkBest(double fPercent)
	{
		ZASSERTSTATE(fPercent >= 0.0);

		std::vector<ScoredLightFrame> lightFrames;

		for (auto & group : imageGroups)
		{
			for (size_t i = 0; i != group.pictures->mydata.size(); ++i)
			{
				const auto& file = group.pictures->mydata[i];
				if (file.IsLightFrame())
				{
					lightFrames.emplace_back(
						static_cast<decltype(ScoredLightFrame::group)>(group.index()),
						static_cast<decltype(ScoredLightFrame::index)>(i),
						file.m_fOverallQuality
					);
				}
			}
			group.setDirty();
		}

		const size_t last = static_cast<size_t>(fPercent * lightFrames.size() / 100.0);
		//
		// Sort in *descending* order (see operator < in class definition)
		std::sort(lightFrames.begin(), lightFrames.end());

		for (size_t i = 0; i < lightFrames.size(); i++)
		{
			const auto id = lightFrames[i].group;
			const auto idx = lightFrames[i].index;

			imageGroups[id].pictures->mydata[idx].m_bChecked =
				(i <= last) ? Qt::Checked : Qt::Unchecked;
			QModelIndex start	{ imageGroups[id].pictures->createIndex(idx, 0) };
			QModelIndex end		{ imageGroups[id].pictures->createIndex(idx, 0) };
			QVector<int> role{ Qt::CheckStateRole };
			imageGroups[id].pictures->dataChanged(start, end, role);
		}
	}

	// Change the name of the specified group
	void FrameList::setGroupName(int id, const QString& name)
	{
		ZASSERTSTATE(id > -1 && id < imageGroups.size());
		if (-1 == id) id = index;	// set to current group

		imageGroups[id].setName(name);
	}

	/* ------------------------------------------------------------------- */

	void FrameList::retranslateUi()
	{
		int i = 0;
		for (auto& group : imageGroups)
		{
			if (0 == i)
				group.Name = QCoreApplication::translate("DSS::StackingDlg", "Main Group", "IDS_MAINGROUP");
			else if (!group.nameChanged)
				group.Name = QCoreApplication::translate("DSS::StackingDlg", "Group %1", "IDS_GROUPIDMASK").arg(i);

			++i;
		}
	}

}