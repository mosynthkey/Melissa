<template>
    <v-app>
        <v-app-bar :elevation="2">
            <template v-slot:prepend>
                <v-app-bar-nav-icon></v-app-bar-nav-icon>
            </template>
            <v-app-bar-title>Melissa</v-app-bar-title>
        </v-app-bar>
        <v-main>
            <v-btn variant="tonal" @click="sayHello">
                Say Hello
            </v-btn>
            <v-btn v-if="showButton" variant="tonal">
                Button
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
const sayHello = Juce.getNativeFunction("sayHello");

const showButton = ref(true);

const toggleButton = () => {
    showButton.value = !showButton.value;
};

// @ts-ignore
const removalToken = window.__JUCE__.backend.addEventListener("helloFromC++", (objectFromBackend) => {
    toggleButton();
});
</script>