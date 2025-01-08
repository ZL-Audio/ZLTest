// Copyright (C) 2025 - zsliu98
// This file is part of ZLDuckerTest
//
// ZLDuckerTest is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLDuckerTest is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLDuckerTest. If not, see <https://www.gnu.org/licenses/>.

#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p)
        : AudioProcessorEditor(&p), processorRef(p) {
    juce::ignoreUnused(processorRef);

    setSize(400, 150);
    getConstrainer()->setFixedAspectRatio(400.f / 150.f);
    setResizable(true, p.wrapperType != PluginProcessor::wrapperType_AudioUnitv3);
}

PluginEditor::~PluginEditor() {
}

void PluginEditor::paint(juce::Graphics &) {
}

void PluginEditor::resized() {
}