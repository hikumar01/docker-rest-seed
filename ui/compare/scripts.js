function updateColourofStrings(result) {
    let updatedStr = "";
    for (let i = 0; i < result.length; i++) {
        const {operation, str} = result[i];
        // const encodedStr = str.replace(/\\/g, '\\\\'); // Escape backslash
        const encodedStr = str.replace(/\\/g, '\\\\').replace(/\n/g, '<br/>'); // Escape backslash & new line
        console.log("str: ", str, ", encodedStr: ", encodedStr);
        let className = "black-text";
        if (operation === "INSERT") {
            className = "green-text";
        } else if (operation === "DELETE") {
            className = "red-text";
        }
        updatedStr += '<span class="' + className + '">' + encodedStr + '</span>&nbsp;';
    }
    return updatedStr;
}

async function compareStrings(event) {
    event.preventDefault();

    const str1 = document.getElementById('left-half').textContent;
    const str2 = document.getElementById('right-half').textContent;
    const encodedStr1 = str1.replace(/\\/g, '\\\\').replace(/\n/g, '<br/>'); // Escape backslash & new line
    const encodedStr2 = str2.replace(/\\/g, '\\\\').replace(/\n/g, '<br/>'); // Escape backslash & new line
    const body_object = { str1: encodedStr1, str2: encodedStr2 };
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
            const updatedStr = updateColourofStrings(result.result);
            console.log("updatedStr: ", updatedStr);
            document.getElementById('result').innerHTML = updatedStr;

            document.getElementById('container').height = '40%';
            document.getElementById('result').height = '55%';
        } else {
            alert('Server error, please try again later.');
        }
    } catch (error) {
        console.error('Error:', error);
    }
}
