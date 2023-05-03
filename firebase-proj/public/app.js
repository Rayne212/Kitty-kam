
var imgRef = storageRef.child('mostRecent/photo.jpg');
firebase.auth().signInAnonymously().then(function() {

  imgRef.getDownloadURL().then(function(url){
    document.querySelector('img').src = url;
  }).catch(function(error) {
    console.error(error);
  });
});

imgRef.getMetadata()
  .then((metadata) => {
    date = new Date(metadata.timeCreated);
    document.getElementById("information").innerHTML = (date.getMonth()+1)+'/'+date.getDate()+'/'+date.getFullYear() + " at " + date.getHours() + ":" + date.getMinutes();
  })
  .catch((error)=> {
    console.error(error);
  });