<template>
    <v-app>
        <v-app-bar :elevation="2">
            <template v-slot:prepend>
                <v-app-bar-nav-icon></v-app-bar-nav-icon>
            </template>
            <v-app-bar-title>Melissa</v-app-bar-title>
        </v-app-bar>
        <v-main>

            <v-btn elevation="2" icon></v-btn>

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
// @ts-ignore
import * as Juce from "juce-framework-frontend";

const excuteCommand = Juce.getNativeFunction("excuteCommand");

// @ts-ignore
const removalToken = window.__JUCE__.backend.addEventListener("MessageFromMelissaModel", (objectFromBackend) => {
    console.log(objectFromBackend);
});
</script>