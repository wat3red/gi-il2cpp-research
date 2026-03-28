#pragma once

#include "../internal_include.h"

namespace MoleMole
{
	struct ConfigScenePoint {
		Unity::Vector3& GetTranPos();
	};

	struct ScenePointData {
		bool isUnlocked;
		ConfigScenePoint* config;
		bool isGroupLimit;
		bool isModelHidden;
		uint32_t entityId;
		uint32_t level;
	};

	struct MapModule {
		static MapModule* Instance();
		Unity::Dictionary<uint32_t, ScenePointData>* GetScenePointDics(uint32_t sceneID);
	};

	struct MapManager {
		static MapManager* Instance();
		uint32_t GetMapSceneID();
	};

	struct UIManager {
		static UIManager* Instance();
		Unity::Camera* GetUICamera();
	};

	struct GeneralMarkData : Il2CppObject {
		uint32_t _sceneID; 
		int32_t _markType; 
		int32_t _iconType; 
		uint32_t _markID;
		float _radius;
		float _sectorAngle;
		float _sectorStartAngle;
		Unity::Vector3 _originPosition;
		System::Nullable<uint32_t>* _worldAreaID;
		System::Nullable<uint32_t>* _subAreaID;
		Unity::Vector3 _areaOffset;
		Unity::Vector3 _indicatorPositionOffset;
		Il2CppString* _questIndicatorIconName;
		bool _hideOnMapAndRadar;
		char _groupId[0x10];
		MoleMole::BaseEntity* _entity;
		bool _hideOnMove;
		bool _hideTrace;
		uint32_t _questSceneId;
		void* _mapMarkPoint;
		bool _hideWhenAreaLocked;
		bool _deleteStopTrack;
		bool _mapLayerDirty;
		uint32_t _mapLayerID;
		void* _monoMarkListenList;
		bool _mapLayerLoading;
		bool _positionDirty;
		void* _OnMapLayerGet;
		void* _hideState;
		bool _isUseGuidePos;
		Unity::Vector3 _guidePosition;
		Unity::Vector3 _guideAreaOffset;
		uint32_t _guideMapLayerID;
	};

	struct MarkManager {
		GeneralMarkData* GetNavigatingMark(uint32_t scene);
		static MarkManager* Instance();
	};

	struct MonoInLevelMapPage {
		Unity::Transform* GetMapBackground();
		Unity::Rect GetMapRect();
	};

	struct InLevelMapPageContext {
		MonoInLevelMapPage* GetPageMono();
		Unity::Rect GetMapViewRect();
	};
}
