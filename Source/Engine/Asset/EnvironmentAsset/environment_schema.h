#pragma once
#include "environment_asset.h"
#include "environment_lighting_schema.h"
#include "environment_post_process_schema.h"

namespace EnvironmentSchema
{
    inline const auto& GetSchema()
    {
        using Module = EnvironmentData;
        static const auto schema = FieldSchema{
            MakeStructField(
                "lighting", "Lighting", &Module::lighting,
                EnvironmentLightingSchema::GetLightingSettingsSchema(),
                DefaultFieldOptions{}),
            MakeStructField(
                "postProcess", "Post Processing", &Module::postProcess,
                EnvironmentPostProcessSchema::GetPostProcessSchema(),
                DefaultFieldOptions{})
        };
        return schema;
    }
}
