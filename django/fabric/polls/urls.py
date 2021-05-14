from django.urls import path
from . import views
from .views import QuestionView
from .views import PollerView
from .views import AnswerTextView
from .views import AnswerChoiceView

app_name = 'polls'

urlpatterns = [
	path('', views.index, name='index'),
	path('poller/', PollerView.as_view()),
	path('question/', QuestionView.as_view()),
	path('answer_text/', AnswerTextView.as_view()),
	path('answer_text/user_id=<int:user_id>', AnswerTextView.as_view()),
	path('answer_choice/', AnswerChoiceView.as_view()),
	path('answer_choice/user_id=<int:user_id>', AnswerChoiceView.as_view()),
]
