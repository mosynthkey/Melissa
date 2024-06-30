<template>
    <v-app>
        <v-app-bar :elevation="2">
            <template v-slot:prepend>
                <v-app-bar-nav-icon></v-app-bar-nav-icon>
            </template>
            <v-app-bar-title>Melissa</v-app-bar-title>
        </v-app-bar>
        <v-main>
            <div class="waveform-container">
                <WaveformView />
            </div>

            <div class="button-container">
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
            </div>
        </v-main>
    </v-app>
</template>

<script setup lang="ts">
import { ref } from 'vue';
import WaveformView from './components/WaveformView.vue';
// @ts-ignore
import * as Juce from "juce-framework-frontend";

const excuteCommand = Juce.getNativeFunction("excuteCommand");

</script>

<style scoped>
.waveform-container {
    margin-top: 20px;
    /* ヘッダーと波形の間に20pxの隙間を作る */
}

.button-container {
    margin-top: 20px;
    /* ボタンと波形の間に隙間を作る */
}
</style>