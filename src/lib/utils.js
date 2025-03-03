
let decodeOggDataPolyfill = null
let nativeDecodeOgg

export async function decodeAudioDataAny(contentType, data, audioContext) {

    if (!data || !data.byteLength) {
        return null
    }

    if ((contentType == "audio/ogg" || contentType == "binary/octet-stream") && !nativeDecodeOgg) {

        if (!decodeOggDataPolyfill) {

            const { decodeOggModule } = await import("./ogg-decode.js")
            decodeOggDataPolyfill = decodeOggModule.call(window)

            console.log("using ogg polyfill")
        }
        const buffer = await decodeOggDataPolyfill.decodeOggData(data)


        return buffer
        
    }
}