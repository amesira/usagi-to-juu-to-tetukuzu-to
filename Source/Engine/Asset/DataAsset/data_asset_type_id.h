// data_asset_type_id.h
// ・DataAssetの種類を表す整数値を定義する（仕組みはComponentTypeIDと同様）
#pragma once

class DataAssetTypeID {
private:
    // 次に割り当てるDataAssetTypeID
    inline static int m_nextTypeId = 0;

public:
    // DataAssetの種類ごとに一意なIDを取得
    template <class T>
    static int getTypeID() {
        static int typeId = m_nextTypeId++;
        return typeId;
    }
};