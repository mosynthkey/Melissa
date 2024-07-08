<template>
  <v-dialog v-model="fileListDialog" fullscreen>
    <v-card>
      <v-toolbar dark color="primary">
        <v-btn icon dark @click="closeFileList">
          <v-icon>mdi-close</v-icon>
        </v-btn>
        <v-toolbar-title>ファイルリスト</v-toolbar-title>
        <v-spacer></v-spacer>
        <v-btn icon dark @click="showFileChooserAndImport">
          <v-icon>mdi-file-import</v-icon>
        </v-btn>
        <v-btn icon dark @click="refreshFileList">
          <v-icon>mdi-refresh</v-icon>
        </v-btn>
        <v-btn icon dark>
          <v-icon>mdi-dots-vertical</v-icon>
        </v-btn>
      </v-toolbar>
      <v-list>
        <v-list-item v-for="(file, index) in fileList" :key="index" @click="loadFileAndCloseDialog(file)">
          <template v-slot:prepend>
            <v-icon>mdi-file-music</v-icon>
          </template>
          <v-list-item-title>{{ getFileName(file) }}</v-list-item-title>
        </v-list-item>
      </v-list>
    </v-card>
  </v-dialog>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';
// @ts-ignore
import * as Juce from "juce-framework-frontend";

const getFileList = Juce.getNativeFunction("getFileList");
const loadFile = Juce.getNativeFunction("loadFile");
const showFileChooserAndImport = Juce.getNativeFunction("showFileChooserAndImport");

const fileListDialog = ref(false);
const fileList = ref<string[]>([]);

const fetchFileList = async () => {
  try {
    const result = await getFileList();
    fileList.value = JSON.parse(result);
  } catch (error) {
    console.error('ファイルリストの取得に失敗しました:', error);
  }
};

const getFileName = (filePath: string) => {
  return filePath.split('/').pop() || filePath;
};

const loadFileAndCloseDialog = (file: string) => {
  try {
    loadFile(file);
    fileListDialog.value = false;
  } catch (error) {
    console.error('ファイルのロードに失敗しました:', error);
  }
};

const closeFileList = () => {
  fileListDialog.value = false;
};

const refreshFileList = () => {
  fetchFileList();
};

onMounted(async () => {
  await fetchFileList();
});

// 親コンポーネントに公開するメソッド
defineExpose({
  showFileList: () => {
    fileListDialog.value = true;
  }
});
</script>
