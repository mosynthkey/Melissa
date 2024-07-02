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
                <v-btn variant="tonal" @click="excuteCommand('SetLoopStart', 1)">
                    Set loop start
                </v-btn>
                <v-btn variant="tonal" @click="excuteCommand('SetLoopEnd', 1)">
                    Set loop end
                </v-btn>
                <v-btn variant="tonal" @click="showFileList">
                    ファイルリスト表示
                </v-btn>
            </div>
        </v-main>

        <!-- ファイルリストポップアップ -->
        <v-dialog v-model="fileListDialog" max-width="500px">
            <v-card>
                <v-card-title>ファイルリスト</v-card-title>
                <v-card-text>
                    <v-list>
                        <v-list-item v-for="(file, index) in fileList" :key="index" @click="openFile(file)">
                            <v-list-item-title>{{ getFileName(file) }}</v-list-item-title>
                        </v-list-item>
                    </v-list>
                </v-card-text>
            </v-card>
        </v-dialog>

        <!-- ローディング画面 -->
        <v-dialog v-model="isLoading" persistent max-width="300px">
            <v-card>
                <v-card-text>
                    ファイルを読み込んでいます...
                    <v-progress-circular indeterminate color="primary"></v-progress-circular>
                </v-card-text>
            </v-card>
        </v-dialog>
    </v-app>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue';
import WaveformView from './components/WaveformView.vue';
// @ts-ignore
import * as Juce from "juce-framework-frontend";

const excuteCommand = Juce.getNativeFunction("excuteCommand");
const getFileList = Juce.getNativeFunction("getFileList");
const loadFile = Juce.getNativeFunction("loadFile");

const fileListDialog = ref(false);
const fileList = ref<string[]>([]);
const isLoading = ref(false);

const fetchFileList = async () => {
    try {
        const result = await getFileList();
        fileList.value = JSON.parse(result);
    } catch (error) {
        console.error('ファイルリストの取得に失敗しました:', error);
    }
};

const showFileList = () => {
    fileListDialog.value = true;
};

const getFileName = (filePath: string) => {
    return filePath.split('/').pop() || filePath;
};

const openFile = (file: string) => {
    isLoading.value = true;
    loadFile(file);
    fileListDialog.value = false;
};

let notificationToken: any = null;

onMounted(() => {
    fetchFileList();

    notificationToken = window.__JUCE__.backend.addEventListener("MelissaNotification", (objectFromBackend: any) => {
        const message = objectFromBackend[0];
        if (message === 'songChanged') {
            isLoading.value = false;
        }
    });
});

onUnmounted(() => {
    if (notificationToken) {
        window.__JUCE__.backend.removeEventListener(notificationToken);
    }
});
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