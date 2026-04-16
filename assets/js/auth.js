document.addEventListener('DOMContentLoaded', () => {
    // DOM Elements
    const toggleOwner = document.getElementById('toggle-owner');
    const toggleOthers = document.getElementById('toggle-others');
    const ownerForms = document.getElementById('owner-forms');
    const othersForms = document.getElementById('others-forms');
    
    const ownerLoginView = document.getElementById('owner-login-view');
    const ownerSignupView = document.getElementById('owner-signup-view');
    const goToSignup = document.getElementById('go-to-signup');
    const goToLogin = document.getElementById('go-to-login');

    // Toggle Tracks (Owner vs Others)
    toggleOwner.addEventListener('click', () => {
        toggleOwner.classList.add('active');
        toggleOthers.classList.remove('active');
        ownerForms.classList.remove('d-none');
        othersForms.classList.add('d-none');
    });

    toggleOthers.addEventListener('click', () => {
        toggleOthers.classList.add('active');
        toggleOwner.classList.remove('active');
        othersForms.classList.remove('d-none');
        ownerForms.classList.add('d-none');
    });

    // Toggle Owner Views (Login vs Signup)
    goToSignup.addEventListener('click', (e) => {
        e.preventDefault();
        ownerLoginView.classList.add('d-none');
        ownerSignupView.classList.remove('d-none');
    });

    goToLogin.addEventListener('click', (e) => {
        e.preventDefault();
        ownerSignupView.classList.add('d-none');
        ownerLoginView.classList.remove('d-none');
    });

    // Form Submissions
    const handleFormSubmit = async (id, type) => {
        const form = document.getElementById(id);
        form.addEventListener('submit', async (e) => {
            e.preventDefault();
            
            const formData = new FormData(form);
            const data = Object.fromEntries(formData.entries());
            
            console.log(`Submitting ${type}:`, data);
            
            // This will talk to our C server at http://localhost:8080/auth
            try {
                const response = await fetch('/auth', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify({
                        action: type,
                        ...data
                    })
                });

                console.log('Response status:', response.status);
                const text = await response.text();
                console.log('Response text:', text);
                
                const result = JSON.parse(text);
                alert(result.message);
                
                if (result.success) {
                    // Handle successful login/signup
                    // window.location.href = '/dashboard';
                }
            } catch (err) {
                console.error('Connection failed:', err);
                alert('Success! (Note: C server is currently being built, this is a placeholder)');
            }
        });
    };

    // Initialize all forms
    handleFormSubmit('owner-login-form', 'owner_login');
    handleFormSubmit('owner-signup-form', 'signup');
    handleFormSubmit('others-login-form', 'staff_login');
});
