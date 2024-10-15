function clearContent(elementId) {
    document.getElementById(elementId).innerHTML = '';
}

function toggleInputElements(enabled) {
    document.getElementById('left-half').enabled = enabled;
    document.getElementById('right-half').enabled = enabled;
    document.getElementById('compareBtn').enabled = enabled;
}

function updateColourofStrings(str1, str2, result) {
    let updatedStr1 = "", updatedStr2 = "";
    let compareStr1 = "", compareStr2 = "";
    for (let i = 0; i < result.length; i++) {
        const {operation, str} = result[i];
        const encodedStr = str.replace(/\\/g, '\\\\'); // Escape backslash
        if (operation === "INSERT") {
            compareStr2 += encodedStr;
            updatedStr2 += '<span class="green-text">' + encodedStr + '</span>';
        } else if (operation === "DELETE") {
            compareStr1 += encodedStr;
            updatedStr1 += '<span class="red-text">' + encodedStr + '</span>';
        } else { // operation === "EQUAL"
            compareStr1 += encodedStr;
            compareStr2 += encodedStr;
            updatedStr1 += encodedStr;
            updatedStr2 += encodedStr;
        }
    }
    if (compareStr1 !== str1) {
        console.log("Error: str1: ", str1, ", compareStr1: ", compareStr1);
    }
    if (compareStr2 !== str2) {
        console.log("Error: str2: ", str2, ", compareStr2: ", compareStr2);
    }
    return { updatedStr1, updatedStr2 };
}

async function compareStrings(event) {
    event.preventDefault();

    toggleInputElements(false);
    const str1 = document.getElementById('left-half').textContent;
    const str2 = document.getElementById('right-half').textContent;
    const body_object = { str1: str1, str2: str2 };
    console.log("body_object: ", body_object);

    try {
        const response = await fetch('http://localhost:8080/compare', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(body_object)
        });

        if (response.ok) {
            const result = await response.json();
            const string_difference = result.result;

            const { updatedStr1, updatedStr2 } = updateColourofStrings(str1, str2, string_difference);
            document.getElementById('left-half').innerHTML = updatedStr1;
            document.getElementById('right-half').innerHTML = updatedStr2;
        } else {
            alert('Server error, please try again later.');
        }
    } catch (error) {
        console.error('Error:', error);
    } finally {
        toggleInputElements(true);
    }
}
