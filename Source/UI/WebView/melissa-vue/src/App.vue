<template>
    <v-app>
        <v-app-bar :elevation="2">
            <template v-slot:prepend>
                <v-app-bar-nav-icon></v-app-bar-nav-icon>
            </template>
            <v-app-bar-title>Melissa</v-app-bar-title>
        </v-app-bar>
        <v-main>
            <WaveformView />

            <v-btn variant="tonal" @click="excuteCommand('StartStop', 1)">
                Play / Stop
            </v-btn>
            <v-btn variant="tonal" @click="excuteCommand('Back', 1)">
                Back
            </v-btn>
            <v-btn variant="tonal" @click="excuteCommand('Pitch_Minus', 1)">
                Pitch -1
            </v-btn>
            <v-btn variant="tonal" @click="excuteCommand('Pitch_Plus', 1)">
                Pitch +1
            </v-btn>

            <v-btn variant="tonal" @click="excuteCommand('Pitch_Reset', 1)">
                Pitch Reset
            </v-btn>

            <v-btn variant="tonal" @click="excuteCommand('SetSpeed_Minus5', 1)">
                Speed -1
            </v-btn>

            <v-btn variant="tonal" @click="excuteCommand('SetSpeed_Plus5', 1)">
                Speed +1
            </v-btn>

            <v-btn variant="tonal" @click="excuteCommand('ResetSpeed', 1)">
                Speed Reset
            </v-btn>

            <v-btn variant="tonal" @click="test()">
                Waveform
            </v-btn>
        </v-main>
        <v-bottom-navigation>
            <v-btn value="recent">
                <v-icon>mdi-history</v-icon>
                <span>Recent</span>
            </v-btn>
            <v-btn value="favorites">
                <v-icon>mdi-heart</v-icon>
                <span>Favorites</span>
            </v-btn>
            <v-btn value="nearby">
                <v-icon>mdi-map-marker</v-icon>
                <span>Nearby</span>
            </v-btn>
        </v-bottom-navigation>
    </v-app>
</template>

<script setup lang="ts">
import { ref } from 'vue';
import WaveformView from './components/WaveformView.vue';
// @ts-ignore
import * as Juce from "juce-framework-frontend";

const excuteCommand = Juce.getNativeFunction("excuteCommand");
const requestWaveform = Juce.getNativeFunction("requestWaveform");

// @ts-ignore
const messageFromMelissaModel = window.__JUCE__.backend.addEventListener("MessageFromMelissaModel", (objectFromBackend) => {
});

// @ts-ignore
const messageFromMelissaDataSource = window.__JUCE__.backend.addEventListener("MessageFromMelissaDataSource", (objectFromBackend) => {
    console.log(objectFromBackend);
});

const test = () => {
    requestWaveform().then((waveform: number[]) => {
        console.log(waveform);
    });
};

</script>