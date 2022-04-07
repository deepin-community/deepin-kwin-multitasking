/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Sian Cao <yinshuiboy@gmail.com>
 *
 * Maintainer: Sian Cao <yinshuiboy@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "multitasking.h"

class MultitaskingPluginFactory : public KWin::EffectPluginFactory
{
    Q_OBJECT
    Q_INTERFACES(KPluginFactory)
    Q_PLUGIN_METADATA(IID EffectPluginFactory_iid FILE "multitasking.json")

public:
    explicit MultitaskingPluginFactory() {}
    ~MultitaskingPluginFactory() {}

    KWin::Effect *createEffect() const override {
        return new MultitaskingEffect;
    }
};

#include "factory.moc"
