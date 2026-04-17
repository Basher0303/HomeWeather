function convertSeconds(seconds) {
    const hours = Math.floor(seconds / 3600);
    const remainingSecondsAfterHours = seconds % 3600;

    const minutes = Math.floor(remainingSecondsAfterHours / 60);
    const remainingSeconds = Math.floor(remainingSecondsAfterHours % 60);

    return {
        hours: hours,
        minutes: minutes,
        seconds: remainingSeconds,
    };
}

function declOfNum(number, titles) {
    const cases = [2, 0, 1, 1, 1, 2];
    return titles[number % 100 > 4 && number % 100 < 20 ? 2 : cases[number % 10 < 5 ? number % 10 : 5]];
}

export { convertSeconds, declOfNum };
